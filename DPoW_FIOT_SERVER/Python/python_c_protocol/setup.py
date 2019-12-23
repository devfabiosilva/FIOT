from distutils.core import setup, Extension
#Sex 20 Dez 2019 18:45:20 -03 
def main():
    setup(name="fiot_raw_data_obj_name",
        version="1.0.0",
        description="Fenix-IoT DPoW Nano cryptocurrency protocol modules for Python 3 using C library",
        author="Fábio Pereira da Silva",
        author_email="fabioegel@gmail.com",
        ext_modules=[Extension("fiot", ["src/module.c", "src/fiot_commands.c", "src/fpyc_err.c"])])

if __name__ == "__main__":
    main()

