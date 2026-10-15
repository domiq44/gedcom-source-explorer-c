#!/usr/bin/env bash
set -Eeuo pipefail

NVM_VERSION="v0.40.3"
PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
NODE_ACTIVATION_REQUIRED=0

print_usage() {
    printf 'Usage: bash scripts/setup-browser-tests.sh [--help]\n'
    printf 'Installs the optional browser-test dependencies on Fedora.\n'
}

fail() {
    printf 'Error: %s\n' "$1" >&2
    exit 1
}

run_as_root() {
    if [[ "$(id -u)" -eq 0 ]]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        fail 'sudo is required to install Fedora packages.'
    fi
}

has_supported_node() {
    local node_version npm_version node_major npm_major

    command -v node >/dev/null 2>&1 || return 1
    command -v npm >/dev/null 2>&1 || return 1

    node_version="$(node --version 2>/dev/null || true)"
    npm_version="$(npm --version 2>/dev/null || true)"
    node_major="${node_version#v}"
    node_major="${node_major%%.*}"
    npm_major="${npm_version%%.*}"

    [[ "$node_major" =~ ^[0-9]+$ && "$npm_major" =~ ^[0-9]+$ ]] || return 1
    (( node_major >= 22 && npm_major >= 10 ))
}

install_node_with_nvm() {
    local nvm_dir install_script

    nvm_dir="${NVM_DIR:-$HOME/.nvm}"
    export NVM_DIR="$nvm_dir"

    if [[ ! -s "$NVM_DIR/nvm.sh" ]]; then
        command -v curl >/dev/null 2>&1 || fail 'curl is required to install nvm.'
        install_script="$(mktemp)"
        if ! curl -fsSL \
            "https://raw.githubusercontent.com/nvm-sh/nvm/${NVM_VERSION}/install.sh" \
            -o "$install_script"; then
            rm -f "$install_script"
            fail 'Could not download the nvm installer.'
        fi
        if ! bash "$install_script"; then
            rm -f "$install_script"
            fail 'Could not install nvm.'
        fi
        rm -f "$install_script"
    fi

    [[ -s "$NVM_DIR/nvm.sh" ]] || fail "nvm was not found at $NVM_DIR/nvm.sh."
    # shellcheck disable=SC1090,SC1091
    if ! source "$NVM_DIR/nvm.sh"; then
        command -v nvm >/dev/null 2>&1 || fail 'Could not load nvm.'
    fi
    nvm install
    nvm use
}

if [[ "${1:-}" == "--help" ]]; then
    print_usage
    exit 0
fi
[[ "$#" -eq 0 ]] || fail "Unknown argument: $1"

[[ -r /etc/os-release ]] || fail 'Cannot identify this operating system.'
# shellcheck disable=SC1091
source /etc/os-release
[[ "${ID:-}" == "fedora" ]] || fail 'This setup script currently supports Fedora only.'
command -v dnf >/dev/null 2>&1 || fail 'dnf was not found.'

printf 'Installing optional browser-test system dependencies...\n'
run_as_root dnf install -y \
    python3 curl ca-certificates \
    alsa-lib at-spi2-atk atk cairo cups-libs dbus-libs fontconfig freetype \
    glib2 gtk3 libdrm libX11 libX11-xcb libxcb libXcomposite libXdamage \
    libXext libXfixes libXrandr libXScrnSaver libxkbcommon libxshmfence \
    mesa-libgbm nspr nss pango

cd "$PROJECT_ROOT"
if has_supported_node; then
    printf 'Using Node.js %s and npm %s.\n' "$(node --version)" "$(npm --version)"
else
    printf 'Installing Node.js selected by .nvmrc...\n'
    install_node_with_nvm
    NODE_ACTIVATION_REQUIRED=1
fi

printf 'Installing npm dependencies...\n'
npm ci

printf 'Installing Playwright Chromium...\n'
npx playwright install chromium

printf '\nSetup complete.\n'
if [[ "$NODE_ACTIVATION_REQUIRED" -eq 1 ]]; then
    printf 'Activate Node.js in this terminal before testing:\n'
    printf '  source "%s/nvm.sh" && nvm use\n' "$NVM_DIR"
fi
printf 'Run the browser tests with: make test-browser\n'