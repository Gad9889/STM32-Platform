import * as fs from "fs";
import * as path from "path";

export interface ProjectInfo {
  isSTM32Project: boolean;
  cmakeFile?: string;
  cubeMXGenerated: boolean;
  halVersion?: string;
  targetFamily?: string;
  hasPlatform: boolean;
  incDir?: string;
  srcDir?: string;
}

export async function detectSTM32Project(
  workspacePath: string
): Promise<ProjectInfo> {
  const info: ProjectInfo = {
    isSTM32Project: false,
    cubeMXGenerated: false,
    hasPlatform: false,
  };

  // Check for CMakeLists.txt
  const cmakePath = path.join(workspacePath, "CMakeLists.txt");
  if (fs.existsSync(cmakePath)) {
    info.cmakeFile = cmakePath;
    const content = fs.readFileSync(cmakePath, "utf8");

    // Look for STM32 indicators
    if (
      content.includes("STM32") ||
      content.includes("stm32") ||
      content.includes("arm-none-eabi") ||
      content.includes("CMAKE_TOOLCHAIN_FILE")
    ) {
      info.isSTM32Project = true;
    }

    // Check if platform already integrated
    if (
      content.includes("stm32_platform") ||
      content.includes("STM32_Platform")
    ) {
      info.hasPlatform = true;
    }

    // Detect STM32 family
    const familyMatch = content.match(/STM32([FHGLWU][0-9])/i);
    if (familyMatch) {
      info.targetFamily = familyMatch[1].toUpperCase();
    }
  }

  // Check for CubeMX generated files
  const mxProjectFile = findFile(workspacePath, /\.ioc$/);
  if (mxProjectFile) {
    info.cubeMXGenerated = true;
    info.isSTM32Project = true;
  }

  // Check for STM32 HAL
  const halFiles = findFile(workspacePath, /stm32[fhglwu][0-9]xx_hal\.h$/i);
  if (halFiles) {
    info.isSTM32Project = true;

    // Try to detect HAL version
    try {
      const halContent = fs.readFileSync(halFiles, "utf8");
      const versionMatch = halContent.match(
        /HAL_VERSION\s+(?:0x)?([0-9A-Fx]+)/i
      );
      if (versionMatch) {
        info.halVersion = versionMatch[1];
      }

      // Detect family from HAL file
      const familyMatch = halFiles.match(/stm32([fhglwu][0-9])xx_hal\.h$/i);
      if (familyMatch) {
        info.targetFamily = familyMatch[1].toUpperCase();
      }
    } catch (e) {
      // Ignore read errors
    }
  }

  // Find Inc and Src directories
  const incDir = findDirectory(workspacePath, [
    "Inc",
    "inc",
    "include",
    "Core/Inc",
  ]);
  const srcDir = findDirectory(workspacePath, [
    "Src",
    "src",
    "source",
    "Core/Src",
  ]);

  if (incDir) info.incDir = incDir;
  if (srcDir) info.srcDir = srcDir;

  return info;
}

function findFile(
  dir: string,
  pattern: RegExp,
  maxDepth: number = 3
): string | null {
  if (maxDepth <= 0) return null;

  try {
    const entries = fs.readdirSync(dir, { withFileTypes: true });

    for (const entry of entries) {
      const fullPath = path.join(dir, entry.name);

      if (entry.isFile() && pattern.test(entry.name)) {
        return fullPath;
      } else if (entry.isDirectory() && !entry.name.startsWith(".")) {
        const found = findFile(fullPath, pattern, maxDepth - 1);
        if (found) return found;
      }
    }
  } catch (e) {
    // Ignore permission errors
  }

  return null;
}

function findDirectory(baseDir: string, names: string[]): string | null {
  for (const name of names) {
    const fullPath = path.join(baseDir, name);
    if (fs.existsSync(fullPath) && fs.statSync(fullPath).isDirectory()) {
      return fullPath;
    }
  }
  return null;
}
