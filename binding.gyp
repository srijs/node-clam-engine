{
  "targets": [
    {
      "target_name": "engine",
      "sources": [
        "engine.cc"
      ],
      "libraries": ["-lclamav"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "<(node_root_dir)/deps/openssl/openssl/include"
      ]
    }
  ]
}
