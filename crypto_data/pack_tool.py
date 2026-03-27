import struct

print("=== Mini-ROT Image Packager ===")

# Firmware payload data to be packaged
payload_data = b"TRUST"
payload_length = len(payload_data) # Automatically calculate length (5 bytes)

# Define firmware header structure
# - Magic Word: File format identifier to validate authenticity
# - Length: Informs STM32 of the actual payload size
magic_word = 0x544F524D  # Hex value for "MROT" characters
header = struct.pack("<I I", magic_word, payload_length)

# Combine header and payload into single binary file
final_bin_file = header + payload_data

# Write packaged firmware to output file
output_filename = "packaged_firmware.bin"
with open(output_filename, "wb") as f:
    f.write(final_bin_file)
print(f"Packaged file created: {output_filename}")
print(f"Header Size: 8 bytes")
print(f"Payload Size: {payload_length} bytes")
print(f"Total File Size: {len(final_bin_file)} bytes")