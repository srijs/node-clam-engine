#include <nan.h>

#include <clamav.h>


using v8::Exception;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::Error;
using Nan::HandleScope;
using Nan::MakeCallback;
using Nan::Null;
using Nan::ObjectWrap;
using Nan::Persistent;
using Nan::Undefined;
using Nan::Utf8String;

static Persistent<FunctionTemplate> constructor;

class ClamScanWorker : public AsyncWorker {

  struct cl_engine *engine;
  int ret;
  const char *filename;
  const char *virname;
  unsigned long int size;

  public:

  ClamScanWorker(Callback *callback, struct cl_engine *engine, Local<String> name)
    : AsyncWorker(callback), engine(engine), size(0) {
      filename = strdup(*Utf8String(name));
    }
  ~ClamScanWorker() {
    delete filename;
  }

  void Execute() {
    ret = cl_scanfile(filename, &virname, &size, engine, CL_SCAN_STDOPT);
  }

  void HandleOKCallback() {
    HandleScope();
    v8::Local<Value>argv[2];
    switch (ret) {
    case CL_CLEAN:
      argv[0] = Null();
      argv[1] = Null();
      break;
    case CL_VIRUS:
      argv[0] = Null();
      argv[1] = Nan::New<String>(virname).ToLocalChecked();
      break;
    default:
      argv[0] = Error(cl_strerror(ret));
      argv[1] = Null();
      break;
    }
    callback->Call(2, argv);
  }

};

class ClamEngine : public ObjectWrap {

  struct cl_engine *engine;

//  // bug in nam.h ?
//  typedef const v8::FunctionCallbackInfo<v8::Value>& FIX_NAM_METHOD_ARGS_TYPE;
//
//  #define FIX_NAM_METHOD(name)                                                       \
//      Nan::NAN_METHOD_RETURN_TYPE name(FIX_NAM_METHOD_ARGS_TYPE info)


  explicit ClamEngine() {
    engine = cl_engine_new();
  }

  public:
    static void Init() {
      Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(ClamEngine::New);
      constructor.Reset(tpl);
      tpl->SetClassName(Nan::New<String>("ClamEngine").ToLocalChecked());
      tpl->InstanceTemplate()->SetInternalFieldCount(1);
      SetPrototypeMethod(tpl, "init", ClamEngine::InitAsync);
      SetPrototypeMethod(tpl, "scanFile", ClamEngine::ScanFileAsync);
    }

    static void New(Nan::NAN_METHOD_ARGS_TYPE info) {
      HandleScope();
      ClamEngine* self = new ClamEngine();
      self->Wrap(info.This());
      info.This()->Set(Nan::New<String>("version").ToLocalChecked(), Nan::New<String>(cl_retver()).ToLocalChecked());
      info.GetReturnValue().Set(info.This());
    }

   static void InitAsync(Nan::NAN_METHOD_ARGS_TYPE info) {
      HandleScope();
      ClamEngine* self = ObjectWrap::Unwrap<ClamEngine>(info.This());
      Callback *callback = new Callback(info[0].As<Function>());
      AsyncQueueWorker(new ClamInitWorker(callback, self->engine));
    }

    static void ScanFileAsync(Nan::NAN_METHOD_ARGS_TYPE info) {
      HandleScope();
      ClamEngine* self = ObjectWrap::Unwrap<ClamEngine>(info.This());
      Local<String> filename = info[0].As<String>();
      Callback *callback = new Callback(info[1].As<Function>());
      AsyncQueueWorker(new ClamScanWorker(callback, self->engine, filename));
    }

  virtual ~ClamEngine() {
    cl_engine_free(engine);
  }

  class ClamInitWorker : public AsyncWorker {

    int ret;
    struct cl_engine *engine;
    unsigned int sigs;

    public:

    ClamInitWorker(Callback *callback, struct cl_engine *engine)
      : AsyncWorker(callback), engine(engine), sigs(0) {}
    ~ClamInitWorker() {}

    void Execute() {
      ret = cl_load(cl_retdbdir(), engine, &sigs, CL_DB_STDOPT);
      if (ret != CL_SUCCESS) {
        return;
      }
      ret = cl_engine_compile(engine);
    }

    void HandleOKCallback() {
      HandleScope();
      Local<Value>argv[2];
      if (ret != CL_SUCCESS) {
        argv[0] = Error(cl_strerror(ret));
        argv[1] = Null();
      } else {
        argv[0] = Null();
        argv[1] = Nan::New<Number>(sigs);
      }
      callback->Call(2, argv);
    }
  };

};

void Init(Handle<Object> exports, Handle<Object> module) {
  assert(cl_init(CL_INIT_DEFAULT) == CL_SUCCESS);
  ClamEngine::Init();
  Local<FunctionTemplate> constructorHandle = New(constructor);
  module->Set(Nan::New<String>("exports").ToLocalChecked(), constructorHandle->GetFunction());
}

NODE_MODULE(engine, Init)
