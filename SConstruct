TARGET = 'linux'

if TARGET == 'mingw32' :
    env = Environment(CC='i486-mingw32-gcc')
    env.AppendUnique(CFLAGS=['-DMINGW', '-DUSE_THREADS', '-lpthreadGC2', '-O3', '-Wall', '-Wno-unused-function', '-fno-strict-aliasing'])
    env.AppendUnique(LINKFLAGS=['-DMINGW', '-DUSE_THREADS', '-lpthreadGC2', '-lm'])
elif TARGET == 'linux' :
    env = Environment()
    env.AppendUnique(CFLAGS=['-DUSE_THREADS', '-O3', '-Wall', '-pthread', '-Wno-unused-function', '-fno-strict-aliasing'])
    env.AppendUnique(LINKFLAGS=['-pthread', '-lm'])

src_c     = ['md4', 'md5', 'nt', 'plaintext', 'chain', 'hash', 'markov', 'bruteforce', 'mask']

sources_c =  map(lambda x: 'src/' + x + '.c', src_c)

binaries = [['markovgen', 'src/markovgen.c'],
            ['rtgen',     'src/rtgen.c'],
            ['rtcrack',   'src/rtcrack.c'],
            ['rtprint',   'src/rtprint.c'],
            ['rtinfo',    'src/rtinfo.c'],
            ['rtextend',  'src/rtextend.c'],
            ['rtverify',  'src/rtverify.c'],
            ['rtmerge',   'src/rtmerge.c']]

if TARGET == 'mingw32' :
    for binary in binaries :
        binary[0] = binary[0] + '.exe'

for binary in binaries :
    env.Program(target=binary[0], source=sources_c+[binary[1]])
