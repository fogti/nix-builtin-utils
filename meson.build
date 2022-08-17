project('Nix standalone builtin utils', 'c', 'cpp',
  default_options: ['cpp_std=c++17'],
  version: '0.2.0',
  license: 'LGPL-2.1',
  meson_version: '>=0.50')

subdir('src')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  libraries  : [libnix_builtins],
  version    : meson.project_version(),
  name       : 'libnix-builtin-utils-2.0',
  filebase   : 'nix-builtin-utils-2.0',
  description: meson.project_name())
