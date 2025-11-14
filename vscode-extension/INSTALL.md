# Installing the STM32 Platform Extension

## For End Users (No npm required)

### Option 1: Install from .vsix file

1. Download the latest `stm32-platform-integration-x.x.x.vsix` file from the [Releases page](https://github.com/Ben-Gurion-Racing/STM32_Platform/releases)

2. Open VS Code

3. Go to Extensions view (`Ctrl+Shift+X`)

4. Click the `...` menu at the top-right of Extensions view

5. Select **Install from VSIX...**

6. Choose the downloaded `.vsix` file

7. Reload VS Code when prompted

### Option 2: Command Line Install

```bash
code --install-extension stm32-platform-integration-x.x.x.vsix
```

## For Developers

### Prerequisites

- Node.js 20.x or higher
- npm 10.x or higher

### Setup

```bash
cd vscode-extension
npm install
```

### Development

```bash
# Compile TypeScript
npm run compile

# Watch mode (auto-recompile on changes)
npm run watch

# Run extension in debug mode
# Press F5 in VS Code (or Run > Start Debugging)
```

### Building .vsix Package

```bash
# Install vsce (VS Code Extension Manager) globally
npm install -g @vscode/vsce

# Build the extension package
npm run package

# Output: stm32-platform-integration-x.x.x.vsix
```

### Publishing

```bash
# Publish to VS Code Marketplace (requires publisher account)
vsce publish

# Or manually upload the .vsix file to:
# https://marketplace.visualstudio.com/manage
```

## Automated Releases

The repository includes a GitHub Actions workflow that automatically:

1. Builds the `.vsix` package on every release tag
2. Attaches the package to the GitHub Release
3. Users can download directly from Releases page

### Creating a Release

```bash
# Tag a new version
git tag v0.1.0
git push origin v0.1.0

# GitHub Actions will automatically build and attach the .vsix file
```

## Troubleshooting

### Extension Not Activating

- Check that your workspace contains a `CMakeLists.txt` file
- The extension only activates for workspaces with CMake projects

### Integration Fails

1. Run: **STM32 Platform: Check Project Compatibility**
2. Verify your project structure matches STM32 CMake requirements
3. Ensure you have `Inc/` and `Src/` directories (or similar)

### Files Already Exist

The extension won't overwrite existing files. If integration was partial:

1. Remove the copied platform files from `Inc/` and `Src/`
2. Remove platform-related lines from `CMakeLists.txt`
3. Try integration again
