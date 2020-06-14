def Settings(**kwargs):
    return {'flags': [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-long-long',
        '-Wno-variadic-macros',
        '-DNDEBUG',
        '-x', 'c++',
    ]}
