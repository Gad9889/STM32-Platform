# VS Code Extension Specification

## Extension: STM32 Platform Integration Assistant

**ID:** `ben-gurion-racing.stm32-platform`  
**Version:** 0.1.0  
**Publisher:** Ben Gurion Racing

---

## Features

### 1. One-Click Platform Integration

**Command:** `STM32 Platform: Add to Project`

**Actions:**

1. Detects STM32CubeMX project (`.ioc` file)
2. Clones/downloads platform files to `Drivers/STM32_Platform/`
3. Updates build system (CMake/Makefile detection)
4. Inserts initialization code in `main.c` user sections
5. Configures IntelliSense paths

**Demo:**

```
Ctrl+Shift+P → "STM32 Platform: Add to Project"
↓
Select CubeMX project file
↓
[✓] Platform files copied
[✓] CMakeLists.txt updated
[✓] main.c initialized
[✓] IntelliSense configured
```

### 2. Template Project Wizard

**Command:** `STM32 Platform: New Project from Template`

**Flow:**

1. Select MCU family (F1/F4/F7/H7)
2. Select peripherals (CAN, UART, SPI, ADC, TIM)
3. Configure pins and clocks
4. Generate CubeMX `.ioc` file
5. Auto-integrate platform
6. Open in VS Code workspace

### 3. Configuration Validator

**Command:** `STM32 Platform: Validate Configuration`

**Actions:**

- Runs `validate_cubemx_config.py` on current `.ioc`
- Shows errors/warnings in Problems panel
- Quick fixes for common issues
- Real-time validation on `.ioc` save

**Output:**

```
Problems (STM32 Platform)
⚠ uart.c:45: UART DMA RX mode should be CIRCULAR
⚠ can.c:23: CAN baudrate not optimal for APB1 clock
✓ 15 checks passed, 2 warnings
```

### 4. Code Snippets

**Trigger:** Type `plt-` in C files

**Snippets:**

- `plt-init` - Full platform initialization
- `plt-can-tx` - CAN message transmission
- `plt-can-rx` - CAN RX callback
- `plt-uart-tx` - UART message send
- `plt-adc-read` - ADC value reading
- `plt-pwm-start` - PWM configuration

**Example:**

```c
// Type: plt-can-tx
plt_CanSendMsg(Can1, &msg);  // Auto-expands with IntelliSense
```

### 5. Build System Generator

**Command:** `STM32 Platform: Generate Build Files`

**Options:**

- CMake (with toolchain file)
- Makefile (ARM GCC)
- tasks.json for VS Code
- launch.json for debugging

**Generated Files:**

```
.vscode/
├── c_cpp_properties.json    # IntelliSense
├── tasks.json               # Build tasks
├── launch.json              # Debug configs
└── settings.json            # Workspace settings

CMakeLists.txt               # Build configuration
arm-none-eabi.cmake          # Toolchain file
```

### 6. Peripheral Status View

**Sidebar Panel:** STM32 Platform

**Display:**

```
STM32 Platform v1.0.0
├── 📡 CAN1 [Initialized]
│   ├── Baudrate: 500 kbit/s
│   ├── Queue: 45/64 messages
│   └── Errors: 0
├── 📨 UART2 [Initialized]
│   ├── Baudrate: 115200
│   ├── TX Queue: 12/32
│   └── RX Queue: 3/32
├── 🔄 SPI1 [Initialized]
└── 📊 ADC1 [Initialized]
    └── Channels: 3 active
```

### 7. Documentation Browser

**Hover:** Function/macro → Shows documentation

**Example:**

```c
plt_CanSendMsg(Can1, &msg);
     ^
     └─ Platform Status: plt_status_t
        Send CAN message through specified channel

        Parameters:
          chanel - CAN channel (Can1/Can2/Can3)
          pData  - Pointer to CAN message

        Returns: HAL_OK or HAL_ERROR

        [View Documentation] [View Example]
```

### 8. Update Manager

**Command:** `STM32 Platform: Check for Updates`

**Actions:**

- Check GitHub releases
- Show changelog
- One-click update
- Backup current version

---

## Installation

### From Marketplace

```
Ctrl+Shift+X → Search "STM32 Platform"
↓
Click "Install"
```

### From VSIX

```bash
code --install-extension stm32-platform-0.1.0.vsix
```

---

## Configuration

### Extension Settings

**File:** `.vscode/settings.json`

```json
{
  "stm32Platform.autoValidate": true,
  "stm32Platform.platformPath": "Drivers/STM32_Platform",
  "stm32Platform.cubemxPath": "C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeMX",
  "stm32Platform.toolchainPath": "C:/Program Files (x86)/GNU Arm Embedded Toolchain",
  "stm32Platform.showPeripheralStatus": true,
  "stm32Platform.enableCodeLens": true
}
```

### Keybindings

```json
{
  "key": "ctrl+alt+p",
  "command": "stm32Platform.addToProject"
},
{
  "key": "ctrl+alt+v",
  "command": "stm32Platform.validateConfig"
}
```

---

## Development Roadmap

### Phase 1: Core Integration (v0.1)

- [ ] Platform file management
- [ ] Build system detection and update
- [ ] Basic code snippets
- [ ] CubeMX validation integration

### Phase 2: Project Wizard (v0.2)

- [ ] Template project generation
- [ ] MCU selection UI
- [ ] Peripheral configuration wizard
- [ ] Auto CubeMX launch

### Phase 3: Advanced Features (v0.3)

- [ ] Real-time peripheral status
- [ ] Interactive documentation
- [ ] Code lens for platform functions
- [ ] Debugging helpers

### Phase 4: Collaboration (v0.4)

- [ ] Share configurations
- [ ] Team templates
- [ ] CI/CD integration
- [ ] Remote debugging support

---

## Technical Implementation

### Extension Structure

```
stm32-platform-extension/
├── package.json              # Extension manifest
├── src/
│   ├── extension.ts          # Entry point
│   ├── commands/
│   │   ├── addToProject.ts
│   │   ├── validateConfig.ts
│   │   └── newProject.ts
│   ├── providers/
│   │   ├── snippetProvider.ts
│   │   ├── hoverProvider.ts
│   │   └── peripheralTreeProvider.ts
│   ├── utils/
│   │   ├── fileManager.ts
│   │   ├── buildSystemDetector.ts
│   │   └── cubemxParser.ts
│   └── views/
│       └── peripheralStatus.ts
├── snippets/
│   └── platform.code-snippets
└── syntaxes/
    └── platform.tmLanguage.json
```

### Dependencies

```json
{
  "activationEvents": [
    "workspaceContains:**/*.ioc",
    "onCommand:stm32Platform.addToProject"
  ],
  "contributes": {
    "commands": [...],
    "snippets": [...],
    "viewsContainers": [...],
    "configuration": [...]
  }
}
```

---

## Contributing

Extension is open source. Contribute at:
`https://github.com/Ben-Gurion-Racing/vscode-stm32-platform`

---

## Alternatives

If VS Code extension not available:

### Python CLI Tool

```bash
pip install stm32-platform-cli

# Add to project
stm32-platform add

# Validate config
stm32-platform validate project.ioc

# Generate build files
stm32-platform build-gen --cmake
```

### Web-Based Generator

Visit: `https://platform.ben-gurion-racing.com/generator`

1. Upload `.ioc` file
2. Select platform version
3. Download generated project with platform integrated

---

## References

- [VS Code Extension API](https://code.visualstudio.com/api)
- [Extension Samples](https://github.com/microsoft/vscode-extension-samples)
- [Publishing Extensions](https://code.visualstudio.com/api/working-with-extensions/publishing-extension)
