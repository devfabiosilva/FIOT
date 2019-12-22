from distutils.core import setup, Extension
#Sex 20 Dez 2019 18:45:20 -03 
def main():
    setup(name="fiot_raw_data_obj_name",
        version="1.0.0",
        description="FIOT protocol Python interface using C library",
        author="Fábio Pereira da Silva",
        author_email="fabioegel@gmail.com",
        ext_modules=[Extension("fiot", ["src/module.c", "src/fiot_commands.c"])])

if __name__ == "__main__":
    main()

