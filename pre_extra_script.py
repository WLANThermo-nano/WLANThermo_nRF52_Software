import shutil

print("Replace variant.h file")
shutil.copyfile("./src/variant_feather_nrf52832.h", "./.core/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832/variant.h")
shutil.copyfile("./src/variant_feather_nrf52840_express.h", "./.core/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52840_express/variant.h")
