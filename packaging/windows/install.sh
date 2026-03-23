#!/bin/bash
# Windows Installer Script for MultiQC C++

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="${PROGRAMFILES}\\MultiQC-CPP"
UNINSTALL_FILE="${INSTALL_DIR}\\uninstall.exe"

echo "==================================="
echo "MultiQC C++ Installer"
echo "==================================="
echo ""

# Check if running as administrator
if ! powershell -Command "([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)" | grep -q "True"; then
    echo "Please run as administrator"
    exit 1
fi

# Create installation directory
echo "Installing to: ${INSTALL_DIR}"
mkdir -p "${INSTALL_DIR}"

# Copy files
echo "Copying files..."
cp -r "${SCRIPT_DIR}/bin" "${INSTALL_DIR}/"
cp -r "${SCRIPT_DIR}/share" "${INSTALL_DIR}/"

# Add to PATH
echo "Adding to system PATH..."
CURRENT_PATH=$(reg query "HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment" /v Path | grep -o '[^ ]*$')
if [[ "${CURRENT_PATH}" != *"${INSTALL_DIR}\\bin"* ]]; then
    setx /M PATH "${CURRENT_PATH};${INSTALL_DIR}\\bin"
fi

# Create uninstaller
echo "Creating uninstaller..."
cat > "${UNINSTALL_FILE}" << 'EOF'
#!/bin/bash
echo "Uninstalling MultiQC C++..."
rm -rf "${PROGRAMFILES}\\MultiQC-CPP"
echo "Uninstallation complete"
EOF

echo ""
echo "==================================="
echo "Installation complete!"
echo ""
echo "MultiQC C++ has been installed to:"
echo "  ${INSTALL_DIR}"
echo ""
echo "Run 'mqc --help' to get started"
echo "==================================="
