
#!/bin/bash
set -euo pipefail

VERSION="1.1.9"
PREFIX="$HOME/.local"
TEMP="$(mktemp -d)"

trap 'rm -rf "$TEMP"' EXIT

echo "=== MariaDB Connector/C++ Local Installer ==="

# Detect Ubuntu version
. /etc/os-release

case "${VERSION_CODENAME:-}" in
    noble)
        UBUNTU_VERSION="24.04"
        ;;
    jammy)
        UBUNTU_VERSION="22.04"
        ;;
    *)
        echo "ERROR: Unsupported Ubuntu version: ${VERSION_CODENAME:-unknown}"
        exit 1
        ;;
esac

# Detect architecture
case "$(dpkg --print-architecture)" in
    amd64)
        ARCH="amd64"
        ;;
    arm64)
        ARCH="aarch64"
        ;;
    *)
        echo "ERROR: Unsupported architecture."
        exit 1
        ;;
esac

# Download official MariaDB binary
BASE_URL="https://dlm.mariadb.com/Connectors/cpp/connector-cpp-${VERSION}/ubuntu-${UBUNTU_VERSION}-${ARCH}"

FILE="mariadb-connector-cpp-${VERSION}-ubu${UBUNTU_VERSION}.tar.gz"

echo "Ubuntu: $UBUNTU_VERSION"
echo "Architecture: $ARCH"
echo "Downloading MariaDB Connector/C++..."

curl -fL --retry 3 "$BASE_URL/$FILE" -o "$TEMP/$FILE"

# Extract
mkdir "$TEMP/extracted"

echo "Extracting..."
tar -xzf "$TEMP/$FILE" -C "$TEMP/extracted"

# Find header
HEADER=$(find "$TEMP/extracted" \
    -type f -path '*/include/mariadb/conncpp.hpp' \
    -print -quit)

if [ -z "$HEADER" ]; then
    echo "ERROR: conncpp.hpp not found."
    exit 1
fi

SRC_INCLUDE="${HEADER%/mariadb/conncpp.hpp}"
SRC_ROOT="${SRC_INCLUDE%/include}"

# Find shared library
LIBRARY=$(find "$SRC_ROOT" \
    -name 'libmariadbcpp.so*' \
    -type f -print -quit)

if [ -z "$LIBRARY" ]; then
    echo "ERROR: MariaDB C++ library not found."
    exit 1
fi

SRC_LIB="$(dirname "$LIBRARY")"

# Install locally
mkdir -p "$PREFIX/include" "$PREFIX/lib/mariadb"

echo "Installing headers..."
cp -a "$SRC_INCLUDE/mariadb" "$PREFIX/include/"

echo "Installing libraries..."
cp -a "$SRC_LIB/." "$PREFIX/lib/mariadb/"

# Configure environment
cat > "$PREFIX/mariadb-env.sh" <<EOF
export MARIADB_CPP_HOME="$PREFIX"
export CPATH="$PREFIX/include:\${CPATH:-}"
export CPLUS_INCLUDE_PATH="$PREFIX/include:\${CPLUS_INCLUDE_PATH:-}"
export LIBRARY_PATH="$PREFIX/lib/mariadb:\${LIBRARY_PATH:-}"
export LD_LIBRARY_PATH="$PREFIX/lib/mariadb:\${LD_LIBRARY_PATH:-}"
EOF

# Add configuration to .bashrc
if ! grep -q 'mariadb-env.sh' "$HOME/.bashrc" 2>/dev/null; then
    echo '' >> "$HOME/.bashrc"
    echo '# MariaDB Connector/C++' >> "$HOME/.bashrc"
    echo '[ -f "$HOME/.local/mariadb-env.sh" ] && source "$HOME/.local/mariadb-env.sh"' >> "$HOME/.bashrc"
fi

# Activate now
source "$PREFIX/mariadb-env.sh"

echo ""
echo "========================================"
echo " MariaDB Connector/C++ installed!"
echo "========================================"
echo ""
echo "Header:"
echo "$PREFIX/include/mariadb/conncpp.hpp"
echo ""
echo "Library:"
echo "$PREFIX/lib/mariadb/libmariadbcpp.so"
echo ""
echo "Activate with:"
echo "source ~/.bashrc"
echo ""
echo "Compile with:"
echo "g++ main.cpp -o main -I$PREFIX/include -L$PREFIX/lib/mariadb -Wl,-rpath,$PREFIX/lib/mariadb -lmariadbcpp"
echo ""
echo "Checking MariaDB Connector/C dependency..."

if ldconfig -p 2>/dev/null | grep -q 'libmariadb.so'; then
    echo "MariaDB Connector/C appears to be installed."
else
    echo "WARNING: MariaDB Connector/C may be missing."
    echo "Connector/C 3.3.3 or newer is required."
fi

echo ""
echo "Installation script finished."

