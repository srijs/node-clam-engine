#include <nan.h>

#include <clamav.h>

using node::ObjectWrap;

using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

static Persistent<FunctionTemplate> constructor;

class ClamScanWorker : public NanAsyncWorker {

  struct cl_engine *engine;
  int ret;
  Persistent<String> filename;
  const char *virname;
  unsigned long int size;

  public:

  ClamScanWorker(NanCallback *callback, struct cl_engine *engine, Local<String> name)
    : NanAsyncWorker(callback), engine(engine), size(0) {
      NanAssignPersistent(filename, name);
    }
  ~ClamScanWorker() {
    NanDisposePersistent(filename);
  }

  void Execute() {
    const char *name = *NanAsciiString(filename);
    ret = cl_scanfile(name, &virname, &size, engine, CL_SCAN_STDOPT);
  }

  void HandleOKCallback() {
    NanScope();
    Local<Value>argv[2];
    switch (ret) {
    case CL_CLEAN:
      argv[0] = NanNull();
      argv[1] = NanNull();
      break;
    case CL_VIRUS:
      argv[0] = NanNull();
      argv[1] = NanNew<String>(virname);
      break;
    default:
      argv[0] = NanError(cl_strerror(ret), ret);
      argv[1] = NanNull();
      break;
    }
    callback->Call(2, argv);
  }

};

class ClamEngine : public ObjectWrap {

  struct cl_engine *engine;

  explicit ClamEngine() {
    engine = cl_engine_new();
  }

  static NAN_METHOD(New) {
    NanScope();
    ClamEngine* self = new ClamEngine();
    self->Wrap(args.This());
    args.This()->Set(NanNew<String>("version"), NanNew<String>(cl_retver()));
    NanReturnValue(args.This());
  }

  static NAN_METHOD(InitAsync) {
    NanScope();
    ClamEngine* self = ObjectWrap::Unwrap<ClamEngine>(args.This());
    NanCallback *callback = new NanCallback(args[0].As<Function>());
    NanAsyncQueueWorker(new ClamInitWorker(callback, self->engine));
    NanReturnUndefined();
  }

  static NAN_METHOD(ScanFileAsync) {
    NanScope();
    ClamEngine* self = ObjectWrap::Unwrap<ClamEngine>(args.This());
    Local<String> filename = args[0].As<String>();
    NanCallback *callback = new NanCallback(args[1].As<Function>());
    NanAsyncQueueWorker(new ClamScanWorker(callback, self->engine, filename));
    NanReturnUndefined();
  }

  virtual ~ClamEngine() {
    cl_engine_free(engine);
  }

  public:
  static void Init() {
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(ClamEngine::New);
    NanAssignPersistent(constructor, tpl);
    tpl->SetClassName(NanNew<String>("ClamEngine"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "init", ClamEngine::InitAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "scanFile", ClamEngine::ScanFileAsync);
  }

  class ClamInitWorker : public NanAsyncWorker {

    int ret;
    struct cl_engine *engine;
    unsigned int sigs;

    public:

    ClamInitWorker(NanCallback *callback, struct cl_engine *engine)
      : NanAsyncWorker(callback), engine(engine), sigs(0) {}
    ~ClamInitWorker() {}

    void Execute() {
      ret = cl_load(cl_retdbdir(), engine, &sigs, CL_DB_STDOPT);
      if (ret != CL_SUCCESS) {
        return;
      }
      ret = cl_engine_compile(engine);
    }

    void HandleOKCallback() {
      NanScope();
      Local<Value>argv[2];
      if (ret != CL_SUCCESS) {
        argv[0] = NanError(cl_strerror(ret), ret);
        argv[1] = NanNull();
      } else {
        argv[0] = NanNull();
        argv[1] = NanNew<Number>(sigs);
      }
      callback->Call(2, argv);
    }
  };

};

void Init(Handle<Object> exports, Handle<Object> module) {
  assert(cl_init(CL_INIT_DEFAULT) == CL_SUCCESS);
  ClamEngine::Init();
  Local<FunctionTemplate> constructorHandle = NanNew(constructor);
  module->Set(NanNew<String>("exports"), constructorHandle->GetFunction());
}

NODE_MODULE(engine, Init)
