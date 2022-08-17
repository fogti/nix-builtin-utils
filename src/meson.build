install_headers('nix-builtin-utils.h')

libnix_builtins = shared_library('nix-builtin-utils-2.0',
  files(
    'list.c',
    'regex.cxx'
  ),
  version : '0.2.0',
  c_args : ['-fno-plt', '-Werror=return-type'],
  cpp_args : ['-fno-plt', '-fno-rtti', '-Werror=return-type'],
  include_directories : include_directories('.'),
  install : true)
