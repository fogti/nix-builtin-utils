install_headers('nix-builtin-utils-2.h')

libnix_bltius = shared_library('nix-builtin-utils-2.0',
  files(
    'list.c',
    'regex.cxx'
  ),
  version : '0.2.0',
  c_args : ['-fno-plt', '-Werror=return-type'],
  cpp_args : ['-fno-plt', '-fno-rtti', '-Werror=return-type'],
  include_directories : include_directories('.'),
  install : true)

if gtest_dep.found()
  test('tests', executable('nbu-tests', 'tests.cxx', dependencies : [gtest_dep], link_with : [libnix_bltius]))
endif
