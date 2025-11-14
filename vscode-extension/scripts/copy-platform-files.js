const fs = require("fs");
const path = require("path");

// Copy platform Inc/ and Src/ directories into the extension
const platformRoot = path.resolve(__dirname, "..", "..");
const extensionRoot = path.resolve(__dirname, "..");

const incSrc = path.join(platformRoot, "Inc");
const incDest = path.join(extensionRoot, "platform", "Inc");

const srcSrc = path.join(platformRoot, "Src");
const srcDest = path.join(extensionRoot, "platform", "Src");

const testKitSrc = path.join(platformRoot, "test-kit");
const testKitDest = path.join(extensionRoot, "test-kit");

function copyDir(src, dest) {
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }

  const entries = fs.readdirSync(src, { withFileTypes: true });

  for (const entry of entries) {
    const srcPath = path.join(src, entry.name);
    const destPath = path.join(dest, entry.name);

    if (entry.isDirectory()) {
      copyDir(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
      console.log(`Copied: ${entry.name}`);
    }
  }
}

console.log("Copying platform files into extension...");
copyDir(incSrc, incDest);
copyDir(srcSrc, srcDest);
if (fs.existsSync(testKitSrc)) {
  copyDir(testKitSrc, testKitDest);
}
console.log("Platform files copied successfully!");
