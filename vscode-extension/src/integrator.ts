import * as fs from "fs";
import * as path from "path";
import { ProjectInfo } from "./detector";

export interface IntegrationOptions {
  peripherals: string[];
  useNewAPI: boolean;
  projectInfo: ProjectInfo;
}

export interface IntegrationResult {
  success: boolean;
  filesAdded: number;
  modifiedFiles: string[];
  exampleFile?: string;
  error?: string;
}

export async function integratePlatform(
  workspacePath: string,
  options: IntegrationOptions
): Promise<IntegrationResult> {
  const result: IntegrationResult = {
    success: false,
    filesAdded: 0,
    modifiedFiles: [],
  };

  try {
    // Determine where to place files
    const incDir =
      options.projectInfo.incDir || path.join(workspacePath, "Inc");
    const srcDir =
      options.projectInfo.srcDir || path.join(workspacePath, "Src");

    // Create directories if they don't exist
    if (!fs.existsSync(incDir)) {
      fs.mkdirSync(incDir, { recursive: true });
    }
    if (!fs.existsSync(srcDir)) {
      fs.mkdirSync(srcDir, { recursive: true });
    }

    // Platform directory (bundled in extension)
    const extensionDir = __dirname;
    const platformRoot = path.resolve(extensionDir, "..", "platform");
    const platformIncDir = path.join(platformRoot, "Inc");
    const platformSrcDir = path.join(platformRoot, "Src");

    // Copy core platform files
    const coreFiles = [
      "platform.h",
      "platform_status.h",
      "platform_config.h",
      "utils.h",
    ];

    const coreSrcFiles = ["platform.c", "platform_config.c", "utils.c"];

    // Copy peripheral files based on selection
    const peripheralMap: {
      [key: string]: { headers: string[]; sources: string[] };
    } = {
      CAN: {
        headers: ["can.h", "database.h", "hashtable.h"],
        sources: ["can.c", "database.c", "hashtable.c"],
      },
      UART: { headers: ["uart.h"], sources: ["uart.c"] },
      SPI: { headers: ["spi.h"], sources: ["spi.c"] },
      ADC: { headers: ["adc.h"], sources: ["adc.c"] },
      TIM: { headers: ["tim.h"], sources: ["tim.c"] },
    };

    let filesToCopy = {
      headers: [...coreFiles],
      sources: [...coreSrcFiles],
    };

    // Add new API if selected
    if (options.useNewAPI) {
      filesToCopy.headers.push("stm32_platform.h");
      filesToCopy.sources.push("stm32_platform.c");
    }

    // Add peripheral-specific files
    for (const peripheral of options.peripherals) {
      if (peripheralMap[peripheral]) {
        filesToCopy.headers.push(...peripheralMap[peripheral].headers);
        filesToCopy.sources.push(...peripheralMap[peripheral].sources);
      }
    }

    // Copy header files
    for (const file of filesToCopy.headers) {
      const src = path.join(platformIncDir, file);
      const dest = path.join(incDir, file);
      if (fs.existsSync(src)) {
        fs.copyFileSync(src, dest);
        result.filesAdded++;
      }
    }

    // Copy source files
    for (const file of filesToCopy.sources) {
      const src = path.join(platformSrcDir, file);
      const dest = path.join(srcDir, file);
      if (fs.existsSync(src)) {
        fs.copyFileSync(src, dest);
        result.filesAdded++;
      }
    }

    // Create DbSetFunctions stub if CAN is enabled
    if (options.peripherals.includes("CAN")) {
      await createDbSetFunctionsStub(incDir, srcDir);
      result.filesAdded += 2;
    }

    // Update CMakeLists.txt
    if (options.projectInfo.cmakeFile) {
      await updateCMakeLists(
        options.projectInfo.cmakeFile,
        filesToCopy,
        incDir,
        srcDir,
        options
      );
      result.modifiedFiles.push(options.projectInfo.cmakeFile);
    }

    // Create example file
    const exampleFile = await createExampleFile(srcDir, options);
    if (exampleFile) {
      result.exampleFile = exampleFile;
      result.filesAdded++;
    }

    result.success = true;
  } catch (error) {
    result.success = false;
    result.error = error instanceof Error ? error.message : String(error);
  }

  return result;
}

async function updateCMakeLists(
  cmakeFile: string,
  files: { headers: string[]; sources: string[] },
  incDir: string,
  srcDir: string,
  options: IntegrationOptions
): Promise<void> {
  let content = fs.readFileSync(cmakeFile, "utf8");

  // Check if already has platform sources
  if (content.includes("# STM32 Platform Sources")) {
    return; // Already integrated
  }

  // Find the project sources section
  const sourcesMatch = content.match(
    /(?:add_executable|target_sources)\s*\([^)]*\)/s
  );

  if (sourcesMatch) {
    // Build source list
    let allSources = [...files.sources];
    
    // Add DbSetFunctions.c if CAN is enabled
    if (options.peripherals.includes("CAN")) {
      allSources.push("DbSetFunctions.c");
    }
    
    const sourcesList = allSources
      .map((f) => `    ${path.basename(srcDir)}/${f}`)
      .join("\n");

    const platformSection = `
# STM32 Platform Sources
set(PLATFORM_SOURCES
${sourcesList}
)
`;

    // Insert before the target definition
    const insertPos = sourcesMatch.index!;
    content =
      content.slice(0, insertPos) + platformSection + content.slice(insertPos);

    // Add platform sources to target
    content = content.replace(
      /add_executable\s*\(\s*\$\{PROJECT_NAME\}([^)]*)\)/,
      "add_executable(${PROJECT_NAME}$1 ${PLATFORM_SOURCES})"
    );
  }

  // Add include directory
  const incDirRelative = path.basename(incDir);
  if (!content.includes(`include_directories(${incDirRelative})`)) {
    const includeSection = `\n# STM32 Platform Include Directory\ninclude_directories(${incDirRelative})\n`;

    // Insert after project() command
    content = content.replace(/project\s*\([^)]*\)/, `$&${includeSection}`);
  }

  // Add compile definitions for enabled peripherals
  const definitions = options.peripherals.map((p) => `PLT_USE_${p}`).join(" ");
  if (!content.includes("# STM32 Platform Definitions")) {
    const defSection = `\n# STM32 Platform Definitions\nadd_compile_definitions(${definitions})\n`;
    content += defSection;
  }

  fs.writeFileSync(cmakeFile, content, "utf8");
}

async function createExampleFile(
  srcDir: string,
  options: IntegrationOptions
): Promise<string | null> {
  const examplePath = path.join(srcDir, "platform_example.c");

  // Don't overwrite if exists
  if (fs.existsSync(examplePath)) {
    return null;
  }

  const useNewAPI = options.useNewAPI;
  const peripherals = options.peripherals;

  let exampleCode = `/**
 * @file platform_example.c
 * @brief STM32 Platform integration example
 * @note Generated by STM32 Platform Integration extension
 */

#include "${useNewAPI ? "stm32_platform.h" : "platform.h"}"
`;

  if (!useNewAPI) {
    peripherals.forEach((p) => {
      exampleCode += `#include "${p.toLowerCase()}.h"\n`;
    });
  }

  exampleCode += `\n/* External HAL handles - declare these in your main.c */\n`;
  if (peripherals.includes("CAN"))
    exampleCode += `extern CAN_HandleTypeDef hcan1;\n`;
  if (peripherals.includes("UART"))
    exampleCode += `extern UART_HandleTypeDef huart2;\n`;
  if (peripherals.includes("SPI"))
    exampleCode += `extern SPI_HandleTypeDef hspi1;\n`;
  if (peripherals.includes("ADC"))
    exampleCode += `extern ADC_HandleTypeDef hadc1;\n`;
  if (peripherals.includes("TIM"))
    exampleCode += `extern TIM_HandleTypeDef htim2;\n`;

  if (useNewAPI) {
    exampleCode += `
void platform_example_init(void) {
    // Initialize platform with selected peripherals
    Platform.begin(${peripherals.includes("CAN") ? "&hcan1" : "NULL"},
                   ${peripherals.includes("UART") ? "&huart2" : "NULL"},
                   ${peripherals.includes("SPI") ? "&hspi1" : "NULL"},
                   ${peripherals.includes("ADC") ? "&hadc1" : "NULL"},
                   ${peripherals.includes("TIM") ? "&htim2" : "NULL"});
    
    P_UART.println("STM32 Platform initialized!");
    P_UART.printf("Version: %s\\n", Platform.version());
}

void platform_example_loop(void) {
`;
    if (peripherals.includes("CAN")) {
      exampleCode += `    // Handle CAN messages\n`;
      exampleCode += `    P_CAN.handleRxMessages();\n`;
      exampleCode += `    if (P_CAN.availableMessages() > 0) {\n`;
      exampleCode += `        P_UART.println("CAN message received");\n`;
      exampleCode += `    }\n\n`;
    }
    if (peripherals.includes("UART")) {
      exampleCode += `    // Handle UART data\n`;
      exampleCode += `    P_UART.handleRxData();\n\n`;
    }
    if (peripherals.includes("ADC")) {
      exampleCode += `    // Read ADC\n`;
      exampleCode += `    uint16_t adc_val = P_ADC.readRaw(0);\n`;
      exampleCode += `    P_UART.printf("ADC: %u\\n", adc_val);\n\n`;
    }
  } else {
    exampleCode += `
void platform_example_init(void) {
    // Initialize platform handlers
    handler_set_t handlers = {
        ${peripherals.includes("CAN") ? ".hcan1 = &hcan1," : ""}
        ${peripherals.includes("UART") ? ".huart2 = &huart2," : ""}
        ${peripherals.includes("SPI") ? ".hspi1 = &hspi1," : ""}
        ${peripherals.includes("ADC") ? ".hadc1 = &hadc1," : ""}
        ${peripherals.includes("TIM") ? ".htim2 = &htim2" : ""}
    };
    plt_SetHandlers(&handlers);
    
    // Initialize peripherals
`;
    if (peripherals.includes("CAN")) exampleCode += `    plt_CanInit(32);\n`;
    if (peripherals.includes("UART")) exampleCode += `    plt_UartInit(32);\n`;
    if (peripherals.includes("SPI")) exampleCode += `    plt_SpiInit(32);\n`;
    if (peripherals.includes("ADC")) exampleCode += `    plt_AdcInit();\n`;
    if (peripherals.includes("TIM")) exampleCode += `    plt_TimInit();\n`;

    exampleCode += `}\n\nvoid platform_example_loop(void) {\n`;
    if (peripherals.includes("CAN"))
      exampleCode += `    plt_CanProcessRxMsgs();\n`;
    if (peripherals.includes("UART"))
      exampleCode += `    plt_UartProcessRxMsgs();\n`;
  }

  exampleCode += `}\n`;

  fs.writeFileSync(examplePath, exampleCode, "utf8");
  return examplePath;
}

async function createDbSetFunctionsStub(
  incDir: string,
  srcDir: string
): Promise<void> {
  const headerPath = path.join(incDir, "DbSetFunctions.h");
  const sourcePath = path.join(srcDir, "DbSetFunctions.c");

  // Don't overwrite if exists
  if (fs.existsSync(headerPath) || fs.existsSync(sourcePath)) {
    return;
  }

  const headerContent = `/**
 * @file DbSetFunctions.h
 * @brief CAN database set functions (user-defined)
 * @note Generated stub - customize for your CAN database
 */

#ifndef DBSETFUNCTIONS_H
#define DBSETFUNCTIONS_H

#include <stdint.h>

/**
 * @brief Initialize database set functions
 * 
 * Register your CAN message handlers here using hash_InsertMember()
 * 
 * @example
 * hash_member_t member = {
 *     .id = 0x123,
 *     .Set_Function = MyHandler
 * };
 * hash_InsertMember(&member);
 */
void DbSetFunctionsInit(void);

/* Add your CAN message handler declarations here */
// void MyHandler(uint8_t *data);

#endif /* DBSETFUNCTIONS_H */
`;

  const sourceContent = `/**
 * @file DbSetFunctions.c
 * @brief CAN database set functions implementation
 */

#include "DbSetFunctions.h"
#include "hashtable.h"

/* Add your CAN message handler implementations here */

void DbSetFunctionsInit(void) {
    /* Register your CAN message handlers here
     * 
     * Example:
     * hash_member_t member = {
     *     .id = 0x123,
     *     .Set_Function = MyHandler
     * };
     * hash_InsertMember(&member);
     */
}
`;

  fs.writeFileSync(headerPath, headerContent, "utf8");
  fs.writeFileSync(sourcePath, sourceContent, "utf8");
}
