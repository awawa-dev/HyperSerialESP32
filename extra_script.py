Import("env")
platform = env.PioPlatform()

import sys
from os.path import join
from pathlib import Path
import shutil

sys.path.append(join(platform.get_package_dir("tool-esptoolpy")))
import esptool

def post_program_action(source, target, env):
    program_path = target[0].get_abspath()
    path = Path(program_path)
    shutil.copy(program_path, path.parent.parent.absolute())

# Combine separate bin files with their respective offsets into a single file
# This single file must then be flashed to an ESP32 node with 0 offset.
# Copied from: https://github.com/letscontrolit/ESPEasy/blob/mega/tools/pio/post_esp32.py
def esp32_create_combined_bin(source, target, env):
    print("Generating combined binary for serial flashing")

    # The offset from begin of the file where the app0 partition starts
    # This is defined in the partition .csv file
    app_offset = 0x10000

    new_file_name = env.subst("$BUILD_DIR/${PROGNAME}.factory.bin")
    sections = env.subst(env.get("FLASH_EXTRA_IMAGES"))
    firmware_name = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    chip = env.get("BOARD_MCU")
    flash_size = env.BoardConfig().get("upload.flash_size")
    flash_freq = env.BoardConfig().get("build.f_flash", '40m')
    flash_freq = flash_freq.replace('000000L', 'm')
    flash_mode = env.BoardConfig().get("build.flash_mode", "dio")
    memory_type = env.BoardConfig().get("build.arduino.memory_type", "qio_qspi")
    if flash_mode == "qio" or flash_mode == "qout":
        flash_mode = "dio"
    if memory_type == "opi_opi" or memory_type == "opi_qspi":
        flash_mode = "dout"
    cmd = [
        "--chip",
        chip,
        "merge_bin",
        "-o",
        new_file_name,
        "--flash_mode",
        flash_mode,
        "--flash_freq",
        flash_freq,
        "--flash_size",
        flash_size,
    ]

    print("    Offset | File")
    for section in sections:
        sect_adr, sect_file = section.split(" ", 1)
        print(f" -  {sect_adr} | {sect_file}")
        cmd += [sect_adr, sect_file]

    print(f" - {hex(app_offset)} | {firmware_name}")
    cmd += [hex(app_offset), firmware_name]

    print('Using esptool.py arguments: %s' % ' '.join(cmd))

    esptool.main(cmd)
    path = Path(new_file_name)
    shutil.copy(new_file_name, path.parent.parent.absolute())

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_program_action)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_combined_bin)
env.Replace(PROGNAME="firmware_%s" % env.GetProjectOption("custom_prog_version"))

