import os
import sys  
import subprocess
import time  
from os.path import join, exists
from colorama import Fore, Style

env = DefaultEnvironment()
platform = env.PioPlatform()
sys.path.append(join(platform.get_package_dir("tool-esptoolpy"))) 
import esptool

MARKER = b'\xDE\xAD\xBE\xEF'
AUDIO_FLASH_OFFSET = 0x200000

def find_wav_file(audio_dir):
    wav_files = [file for file in os.listdir(audio_dir) if file.endswith(".wav")]
    if len(wav_files) == 0:
        return None
    if len(wav_files) > 1:
        print(Fore.RED + "Multiple .wav files found. Please keep only one .wav file in the folder." + Style.RESET_ALL)
        for wav in wav_files:
            print(Fore.RED + f" - {wav}" + Style.RESET_ALL)
        return "multiple"
    return join(audio_dir, wav_files[0])

def convert_wav_to_raw(wav_file, raw_file):
    with open(wav_file, "rb") as wav:
        wav.seek(44)
        with open(raw_file, "wb") as raw:
            raw.write(wav.read())

def append_marker_to_raw(raw_file):
    with open(raw_file, "ab") as raw:
        raw.write(MARKER)
        print(Fore.GREEN + f"Marker ({MARKER.hex()}) appended to {raw_file}" + Style.RESET_ALL)

def esp32_create_combined_bin(source, target, env):


    bootloader_bin = env.subst("$BUILD_DIR/bootloader.bin")
    partitions_bin = env.subst("$BUILD_DIR/partitions.bin")
    firmware_bin = env.subst("$BUILD_DIR/firmware.bin")
    
    audio_dir = join(env.subst("$PROJECT_DIR"), "audio")
    wav_audio = find_wav_file(audio_dir)
    if wav_audio == "multiple":
        return
    if wav_audio is None:
        print(Fore.RED + "No .wav file found in the audio folder." + Style.RESET_ALL)
        return

    raw_audio = join(audio_dir, "raw_audio.raw")
    if exists(raw_audio):
        os.remove(raw_audio)
        print(Fore.GREEN + f"Deleted existing raw file: {raw_audio}" + Style.RESET_ALL)

    convert_wav_to_raw(wav_audio, raw_audio)
    append_marker_to_raw(raw_audio)

    bootloader_size = os.path.getsize(bootloader_bin)
    partitions_size = os.path.getsize(partitions_bin)
    firmware_size = os.path.getsize(firmware_bin)
    raw_audio_size = os.path.getsize(raw_audio)

    print(Fore.GREEN + f"Bootloader size: {bootloader_size} bytes" + Style.RESET_ALL)
    print(Fore.GREEN + f"Partitions size: {partitions_size} bytes" + Style.RESET_ALL)
    print(Fore.GREEN + f"Firmware size: {firmware_size} bytes" + Style.RESET_ALL)
    print(Fore.GREEN + f"Raw audio size: {raw_audio_size} bytes" + Style.RESET_ALL)

    merged_firmware_dir = join(env.subst("$PROJECT_DIR"), "merged_firmware")
    if not exists(merged_firmware_dir):
        os.makedirs(merged_firmware_dir)

    board_name = env.BoardConfig().get("name", "unknown")
    project_name = os.path.basename(env.subst("$PROJECT_DIR"))
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    output_bin = join(merged_firmware_dir, f"{project_name}_{board_name}_{timestamp}.bin") 


    cmd = [
        "--chip", env.get("BOARD_MCU"),
        "merge_bin",
        "-o", output_bin,
        "--flash_mode", env["__get_board_flash_mode"](env),
        "--flash_freq", env["__get_board_f_flash"](env),
        "--flash_size", env.BoardConfig().get("upload.flash_size", "4MB"),
        hex(0x0000), bootloader_bin,  
        hex(0x8000), partitions_bin, 
        hex(0x10000), firmware_bin,
    ]

    print("Merging binaries (bootloader, partitions, firmware)")

    esptool.main(cmd)


    print("Appending raw audio to merged binary")
    with open(output_bin, "ab") as final_bin:
        with open(raw_audio, "rb") as raw_file:
            final_bin.write(raw_file.read())

    print(Fore.GREEN + f"Final combined binary with audio created: {output_bin}" + Style.RESET_ALL)


firmware_dir = join(env.subst("$PROJECT_DIR"), "firmware")
if not exists(firmware_dir):
    os.makedirs(firmware_dir)

env.AddPostAction("$BUILD_DIR/firmware.bin", esp32_create_combined_bin)
