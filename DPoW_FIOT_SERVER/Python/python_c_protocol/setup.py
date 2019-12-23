#
#	AUTHOR: Fábio Pereira da Silva
#	YEAR: 2019
#	LICENSE: MIT
#	EMAIL: fabioegel@gmail.com or fabioegel@protonmail.com
#

# Sexta 20 Dez 2019 18:45:20 -03 

from distutils.core import setup, Extension, os

def main():
    # Python setup and Extension does not compile Assembly objects
    if (os.system("gcc -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fstack-protector-strong -Wformat -Werror=format-security\
        -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC -c src/f_add_bn_288_le_ia64.S -o src/f_add_bn_288_le_ia64.o")):
        raise

    setup(name="fiot_raw_data_obj_name",
        version="1.0.0",
        description="Fenix-IoT DPoW Nano cryptocurrency protocol modules for Python 3 using C library",
        author="Fábio Pereira da Silva",
        author_email="fabioegel@gmail.com",
        maintainer_email="fabioegel@gmail.com",
        ext_modules=[Extension("fiot", ["src/module.c", "src/fiot_commands.c", "src/fpyc_err.c", "src/nano_dpow_server_util.c"],
            library_dirs=['lib'], libraries=['sodium'], extra_objects=['src/f_add_bn_288_le_ia64.o'])])

if __name__ == "__main__":
    main()

