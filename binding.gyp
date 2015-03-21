{
  "targets": [
    {
      "target_name": "engine",
      "sources": [
        "engine.cc"
      ],
      "libraries": ["-lclamav"],
      "include_dirs": ["<!(node -e \"require('nan')\")"]
    }
  ]
}
