#!/bin/bash
# Script to populate GitHub Wiki after it's been initialized

echo "This script will populate your GitHub Wiki"
echo "IMPORTANT: First initialize the wiki by creating the first page manually:"
echo "1. Go to https://github.com/Gad9889/STM32-Platform/wiki"
echo "2. Click 'Create the first page'"
echo "3. Add any content and save"
echo "4. Then run this script"
echo ""
read -p "Have you created the first wiki page? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Please create the first page, then run this script again"
    exit 1
fi

# Clone wiki repository
echo "Cloning wiki repository..."
cd "$(dirname "$0")"
git clone https://github.com/Gad9889/STM32-Platform.wiki.git temp-wiki

if [ $? -ne 0 ]; then
    echo "Error: Could not clone wiki. Make sure you created the first page!"
    exit 1
fi

cd temp-wiki

# Copy wiki files
echo "Copying wiki pages..."
cp ../wiki/Home.md Home.md
cp ../wiki/Getting-Started.md Getting-Started.md
cp ../wiki/API-Reference.md API-Reference.md
cp ../wiki/Architecture.md Architecture.md
cp ../wiki/Examples.md Examples.md

# Commit and push
echo "Pushing to GitHub Wiki..."
git add .
git commit -m "Add complete documentation

- Home page with overview
- Getting Started guide
- Complete API Reference
- Architecture documentation
- Code examples"
git push

cd ..
rm -rf temp-wiki

echo ""
echo "✅ Wiki populated successfully!"
echo "View it at: https://github.com/Gad9889/STM32-Platform/wiki"
