import struct


print("=== Mini-ROT Anti-Rollback Packager ===")


# Firmware payload data to be packaged
payload_data = b"TRUST"
payload_length = len(payload_data)

# Define firmware header structure with anti-rollback protection
# - Magic Word: File format identifier for authenticity validation
# - Version: Firmware version number for anti-rollback protection  
# - Length: Informs STM32 of the actual payload size
magic_word = 0x544F524D  # Hex value for "MROT" characters
fw_version = 3           # Current firmware version 

# Pack header ("<I I I" represents three Little-Endian 32-bit integers)
header = struct.pack("<I I I", magic_word, fw_version, payload_length)
header_size = len(header) # Dynamically get header size (should be 12 bytes)

# Combine header and payload into single binary file
final_bin_file = header + payload_data

# Generate versioned output filename
output_filename = f"fw_v{fw_version}.bin"

with open(output_filename, "wb") as f:
    f.write(final_bin_file)

print(f"[SUCCESS] Packaged file created: {output_filename}")
print(f" -> Header Size:  {header_size} bytes")
print(f" -> Payload Size: {payload_length} bytes")
print(f" -> Total Size:   {len(final_bin_file)} bytes\n")
