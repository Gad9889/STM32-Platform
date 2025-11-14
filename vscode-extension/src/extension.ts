import * as vscode from "vscode";
import { detectSTM32Project } from "./detector";
import { integratePlatform } from "./integrator";

export function activate(context: vscode.ExtensionContext) {
  console.log("STM32 Platform Integration extension activated");

  // Register integrate command
  const integrateCommand = vscode.commands.registerCommand(
    "stm32platform.integrate",
    async () => {
      await handleIntegration();
    }
  );

  // Register compatibility check command
  const checkCommand = vscode.commands.registerCommand(
    "stm32platform.checkCompatibility",
    async () => {
      await checkCompatibility();
    }
  );

  context.subscriptions.push(integrateCommand, checkCommand);

  // Auto-detect on workspace open
  const config = vscode.workspace.getConfiguration("stm32platform");
  if (config.get("autoDetect")) {
    setTimeout(() => autoDetectAndPrompt(), 2000);
  }
}

async function handleIntegration() {
  try {
    const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
    if (!workspaceFolder) {
      vscode.window.showErrorMessage("No workspace folder open");
      return;
    }

    // Detect if this is an STM32 CMake project
    const projectInfo = await detectSTM32Project(workspaceFolder.uri.fsPath);
    if (!projectInfo.isSTM32Project) {
      const answer = await vscode.window.showWarningMessage(
        "This does not appear to be an STM32 CMake project. Continue anyway?",
        "Yes",
        "No"
      );
      if (answer !== "Yes") {
        return;
      }
    }

    // Show integration options
    const config = vscode.workspace.getConfiguration("stm32platform");
    const useNewAPI = config.get("useNewAPI", true);
    const defaultPeripherals = config.get<string[]>("defaultPeripherals", [
      "CAN",
      "UART",
    ]);

    const peripherals = await vscode.window.showQuickPick(
      ["CAN", "UART", "SPI", "ADC", "TIM"],
      {
        canPickMany: true,
        placeHolder: "Select peripherals to enable",
        title: "STM32 Platform Integration",
      }
    );

    if (!peripherals || peripherals.length === 0) {
      vscode.window.showInformationMessage("Integration cancelled");
      return;
    }

    // Perform integration
    await vscode.window.withProgress(
      {
        location: vscode.ProgressLocation.Notification,
        title: "Integrating STM32 Platform",
        cancellable: false,
      },
      async (progress) => {
        progress.report({ increment: 0, message: "Analyzing project..." });

        const result = await integratePlatform(workspaceFolder.uri.fsPath, {
          peripherals,
          useNewAPI,
          projectInfo,
        });

        if (result.success) {
          progress.report({ increment: 100, message: "Complete!" });

          const message =
            `STM32 Platform integrated successfully!\n` +
            `- Added ${result.filesAdded} files\n` +
            `- Updated CMakeLists.txt\n` +
            `- Enabled: ${peripherals.join(", ")}`;

          const action = await vscode.window.showInformationMessage(
            message,
            "View Changes",
            "Open Example"
          );

          if (action === "View Changes" && result.modifiedFiles.length > 0) {
            const uri = vscode.Uri.file(result.modifiedFiles[0]);
            await vscode.window.showTextDocument(uri);
          } else if (action === "Open Example" && result.exampleFile) {
            const uri = vscode.Uri.file(result.exampleFile);
            await vscode.window.showTextDocument(uri);
          }
        } else {
          vscode.window.showErrorMessage(`Integration failed: ${result.error}`);
        }
      }
    );
  } catch (error) {
    vscode.window.showErrorMessage(`Integration error: ${error}`);
  }
}

async function checkCompatibility() {
  const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
  if (!workspaceFolder) {
    vscode.window.showErrorMessage("No workspace folder open");
    return;
  }

  const projectInfo = await detectSTM32Project(workspaceFolder.uri.fsPath);

  let message = `**Project Type:** ${
    projectInfo.isSTM32Project ? "STM32 CMake" : "Unknown"
  }\n`;
  message += `**Compatible:** ${
    projectInfo.isSTM32Project ? "✓ Yes" : "✗ No"
  }\n\n`;

  if (projectInfo.cmakeFile) {
    message += `**CMakeLists.txt:** Found\n`;
  }
  if (projectInfo.cubeMXGenerated) {
    message += `**CubeMX Generated:** Yes\n`;
  }
  if (projectInfo.halVersion) {
    message += `**HAL Version:** ${projectInfo.halVersion}\n`;
  }
  if (projectInfo.targetFamily) {
    message += `**STM32 Family:** ${projectInfo.targetFamily}\n`;
  }

  const panel = vscode.window.createWebviewPanel(
    "compatibility",
    "STM32 Platform Compatibility",
    vscode.ViewColumn.One,
    {}
  );

  panel.webview.html = `
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body { font-family: var(--vscode-font-family); padding: 20px; }
                h1 { color: var(--vscode-foreground); }
                pre { background: var(--vscode-textCodeBlock-background); padding: 10px; }
            </style>
        </head>
        <body>
            <h1>Compatibility Check</h1>
            <pre>${message}</pre>
        </body>
        </html>
    `;
}

async function autoDetectAndPrompt() {
  const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
  if (!workspaceFolder) {
    return;
  }

  const projectInfo = await detectSTM32Project(workspaceFolder.uri.fsPath);
  if (projectInfo.isSTM32Project && !projectInfo.hasPlatform) {
    const answer = await vscode.window.showInformationMessage(
      "STM32 CMake project detected. Would you like to integrate BGU Racing Platform?",
      "Integrate",
      "Not Now",
      "Don't Ask Again"
    );

    if (answer === "Integrate") {
      vscode.commands.executeCommand("stm32platform.integrate");
    } else if (answer === "Don't Ask Again") {
      const config = vscode.workspace.getConfiguration("stm32platform");
      await config.update(
        "autoDetect",
        false,
        vscode.ConfigurationTarget.Workspace
      );
    }
  }
}

export function deactivate() {}
