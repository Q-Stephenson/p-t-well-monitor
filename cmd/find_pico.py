import subprocess
import re

def find_pico_comport():
    result = subprocess.run(
        ["wmic", "path", "Win32_PnPEntity", "where", "Name like '%(COM%)'", "get", "Name"],
        capture_output=True, text=True
    )

    matches = re.findall(r'(COM\d+)', result.stdout)

    for line in result.stdout.splitlines():
        if "Pico" in line or "USB Serial Device" in line:
            port_match = re.search(r'\(COM(\d+)\)', line)
            if port_match:
                return f'COM{port_match.group(1)}'
    return None

if __name__ == "__main__":
    port = find_pico_comport()
    if port:
        print(port)
    else:
        print("NOT FOUND")
