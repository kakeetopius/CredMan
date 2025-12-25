project('credman', 'c')

sqlite_dep = dependency('sqlcipher')
incdir = include_directories('include')

src_dir = 'src'
src_commands_dir = 'commands'
src_db_dir = 'db'
src_objects_dir = 'objects'
src_util_dir = 'util'

commands_srcs = ['acc_mgmt.c', 'api_mgmt.c', 'runners.c']
db_srcs = ['acc_access.c', 'api_access.c', 'general.c' ]
objects_srcs = ['acc_obj.c', 'api_obj.c']
util_srcs = ['argparser.c', 'input.c', 'output.c', 'passwd_gen.c']

srcs = ['src/main.c']

foreach src_file : commands_srcs
  src_file = src_dir / src_commands_dir / src_file
  srcs += src_file
endforeach

foreach src_file : db_srcs
  src_file = src_dir / src_db_dir / src_file
  srcs += src_file
endforeach

foreach src_file : objects_srcs
  src_file = src_dir / src_objects_dir / src_file
  srcs += src_file
endforeach

foreach src_file : util_srcs
  src_file = src_dir / src_util_dir / src_file
  srcs += src_file
endforeach

executable(
  'cman',
  sources: srcs,
  dependencies: sqlite_dep,
  include_directories: incdir,
  install: true,
)


