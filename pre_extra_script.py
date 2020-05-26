import shutil

print("Replace variant.h file")
shutil.copyfile("./src/variant.h", "./.core/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832/variant.h")
