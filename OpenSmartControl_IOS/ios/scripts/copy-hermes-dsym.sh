#!/bin/bash
# Script to generate/copy Hermes dSYM for archive
# This ensures dSYM is included when exporting to TestFlight

set -e

# Paths
HERMES_FRAMEWORK="${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/hermes.framework"
HERMES_BINARY="${HERMES_FRAMEWORK}/hermes"
DWARF_DSYM_FOLDER="${DWARF_DSYM_FOLDER_PATH}"
HERMES_DSYM_DEST="${DWARF_DSYM_FOLDER}/hermes.framework.dSYM"

# Check if we're in a Release build (needed for archive)
if [ "${CONFIGURATION}" != "Release" ]; then
    echo "Skipping Hermes dSYM generation (not Release build)"
    exit 0
fi

# Create dSYM folder if it doesn't exist
mkdir -p "${DWARF_DSYM_FOLDER}"

# Check if Hermes framework exists
if [ ! -d "${HERMES_FRAMEWORK}" ]; then
    echo "Warning: Hermes framework not found at ${HERMES_FRAMEWORK}"
    exit 0
fi

# Check if Hermes binary exists
if [ ! -f "${HERMES_BINARY}" ]; then
    echo "Warning: Hermes binary not found at ${HERMES_BINARY}"
    exit 0
fi

# If dSYM already exists, skip
if [ -d "${HERMES_DSYM_DEST}" ]; then
    echo "Hermes dSYM already exists at ${HERMES_DSYM_DEST}"
    exit 0
fi

# Try to find existing dSYM in various locations
POSSIBLE_DSYM_PATHS=(
    "${PODS_ROOT}/hermes-engine/destroot/Library/Frameworks/universal/hermes.xcframework/ios-arm64/hermes.framework.dSYM"
    "${PODS_ROOT}/hermes-engine/destroot/Library/Frameworks/ios/hermes.framework.dSYM"
    "${PODS_ROOT}/hermes-engine/destroot/Library/Frameworks/universal/hermes.xcframework/*/hermes.framework.dSYM"
)

FOUND_DSYM=""
for DSYM_PATH in "${POSSIBLE_DSYM_PATHS[@]}"; do
    # Handle wildcard paths
    for MATCHED_PATH in ${DSYM_PATH}; do
        if [ -d "${MATCHED_PATH}" ]; then
            FOUND_DSYM="${MATCHED_PATH}"
            break 2
        fi
    done
done

# Copy existing dSYM if found
if [ -n "${FOUND_DSYM}" ] && [ -d "${FOUND_DSYM}" ]; then
    echo "Found Hermes dSYM at ${FOUND_DSYM}, copying to archive..."
    cp -R "${FOUND_DSYM}" "${DWARF_DSYM_FOLDER}/"
    echo "Hermes dSYM copied successfully"
    exit 0
fi

# If no dSYM found, generate one from the binary using dsymutil
echo "No existing dSYM found, generating from Hermes binary..."
if command -v dsymutil &> /dev/null; then
    echo "Generating dSYM for ${HERMES_BINARY}..."
    dsymutil "${HERMES_BINARY}" -o "${HERMES_DSYM_DEST}"
    
    if [ -d "${HERMES_DSYM_DEST}" ]; then
        echo "Hermes dSYM generated successfully at ${HERMES_DSYM_DEST}"
    else
        echo "Warning: Failed to generate Hermes dSYM"
        exit 0
    fi
else
    echo "Warning: dsymutil not found, cannot generate dSYM"
    echo "Hermes dSYM warning may appear during archive/export, but app will still work"
    exit 0
fi

