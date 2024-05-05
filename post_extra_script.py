Import("env", "projenv")

import shutil
import os
import subprocess
import shlex
import binascii
import zipfile
import time

if "nrf52840" in str(env["PIOENV"]):
    sd_param = "0xB6"
    variant = "nrf52840"
else:
    sd_param = "0xB7"
    variant = "nrf52832"


firmware_file = ".pio/build/" + env["PIOENV"] + "/firmware.hex"
dfu_file = ".pio/build/" + env["PIOENV"] + "/app_dfu_package.zip"
out_folder = "out/"
fw_version = env["UNIX_TIME"] #use utc unix time as version

def install_pip(package):
    subprocess.call(["pip", "install", "--upgrade", package])

def call_nrfutil():
    #cmd = "nrfutil pkg generate --hw-version 52 --application-version " + str(fw_version) + " --application " + firmware_file + " --sd-req " + sd_param + " " + dfu_file
    cmd = "adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application-version " + str(fw_version) + " --application " + firmware_file + " --sd-req " + sd_param + " " + dfu_file
    # https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrfutil%2FUG%2Fnrfutil%2Fnrfutil_pkg.html
    # Parameter sd-req bestimmt die Version des Softdevices
    args = shlex.split(cmd)
    subprocess.call(args)

def extract_and_save_header(filename, header_filename, target_folder):
    with zipfile.ZipFile(dfu_file) as myzip:
        with myzip.open(filename) as myfile:
            dat_file = myfile.read()
            dat_file = binascii.hexlify(dat_file).decode("UTF-8").upper()
            dat_file_array = ["0x" + dat_file[i:i + 2] + ", " for i in range(0, len(dat_file), 2)]
            char_array_string = "const uint8_t " + str(header_filename).replace(".", "_") + "[] = {" + str("").join(dat_file_array) + "};"
            with open(target_folder + header_filename, 'w') as f:
                f.write(char_array_string)

def add_version_to_bin_header(header_filename, target_folder, version):
    with open(target_folder + header_filename, 'a') as f:
        f.write("\nconst uint32_t " + str(header_filename).replace(".", "_") + "_version" + " = " + str(version) + "u;")

def after_buildprog(source, target, env):
    print("Generate DFU files")
    #install_pip("nrfutil")
    install_pip("adafruit-nrfutil")
    call_nrfutil()
    if os.path.exists("out") == False:
        os.mkdir("out")
    extract_and_save_header("firmware.dat", "bleFirmwareDat_" + variant + ".h", out_folder)
    extract_and_save_header("firmware.bin", "bleFirmwareBin_" + variant + ".h", out_folder)
    add_version_to_bin_header("bleFirmwareBin_" + variant + ".h", out_folder, fw_version)

env.AddPostAction("buildprog", after_buildprog)
