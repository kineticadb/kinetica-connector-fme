import platform

pluginbuilder_env = Environment()

Import('fme_home')

pluginbuilder_env.Replace(LIBDIR = fme_home.Dir('fmecore'),
                          LDMODULEPREFIX = '',
                          LDMODULESUFFIX = '.so')

pluginbuilder_env.Append(CXXFLAGS = ['--std=c++11'],
                         CCFLAGS = ['-Wall', '-Wno-deprecated',
                                    '-O3', '-fvisibility=hidden'],
                         LIBPATH = ['$LIBDIR'],
                         LIBS = ['fmeobj'])

if platform.uname()[0].lower() == 'linux':
    pluginbuilder_env.Append(CCFLAGS = ['-finline-functions'],
                             CPPDEFINES = ['LINUX'],
                             LINKFLAGS = ['-Wl,-rpath-link,$LIBDIR',
                                          '-z' 'defs'])

elif platform.uname()[0].lower() == 'darwin':
    pluginbuilder_env.Append(CPPDEFINES = ['MACOSX'],
                             CXXFLAGS = ['-stdlib=libc++'],
                             LINKFLAGS = ['-stdlib=libc++',
                                          '-undefined', 'error',
                                          '-multiply_defined', 'suppress',
                                          '-twolevel_namespace'])
else:
    raise SCons.Errors.UserError('Unknown Platform')

Export('pluginbuilder_env', 'fme_home')

pluginbuilder_env.Append(boostLibs = [ "system", "regex", "thread" ])



gpudb_lib          = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/build-RelWithDebInfo/lib/RelWithDebInfo/libgpudb_api_cpp.a')
avro_lib           = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libavrocpp_s.a')
boost_chrono_lib   = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_chrono.a')
boost_dt_lib       = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_date_time.a')
boost_filesys_lib  = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_filesystem.a')
boost_streams_lib  = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_iostreams.a')
boost_progopts_lib = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_program_options.a')
boost_regex_lib    = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_regex.a')
boost_system_lib   = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_system.a') 
boost_thread_lib   = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libboost_thread.a')
snappy_lib         = pluginbuilder_env.File('/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install-enable-abi-static-pic/lib/libsnappy.a')
pthread_lib        = pluginbuilder_env.File('/usr/lib64/libpthread.so')

pluginbuilder_env.Append(LIBS=[gpudb_lib,avro_lib,boost_chrono_lib,boost_dt_lib,boost_filesys_lib,boost_streams_lib,boost_progopts_lib,boost_regex_lib,boost_system_lib,boost_thread_lib,snappy_lib,pthread_lib])

pluginbuilder_env.Append(CPPPATH = [fme_home.Dir('pluginbuilder/cpp'),
                                    fme_home.Dir('fmeobjects/cpp'),
                                    '/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/gpudb',
                                    '/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/gpudb/protocol',
                                    '/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/',
                                    '/home/dkatz/sandboxes/6.0/gpudb-dev/gpudb-api-cpp/_build/thirdparty/install/include',
                                    '/usr/include/',
                                    '/usr/local/boost_1_62_0',
                                    '/usr/local/boost_1_62_0/boost',
                                    '/usr/local/boost_1_62_0/boost/system'])

plugin = pluginbuilder_env.LoadableModule('kinetica',
                                          ['kinetica/geometryvisitor.cpp',
                                           'kinetica/kineticaentrypoints.cpp',
                                           'kinetica/kineticareader.cpp',
                                           'kinetica/kineticawriter.cpp',
                                           'kinetica/kineticautil.cpp'])
                                           
                                           

pluginbuilder_env.Install(fme_home.Dir('plugins'), plugin)
