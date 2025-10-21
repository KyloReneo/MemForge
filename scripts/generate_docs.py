#!/usr/bin/env python3
import os
import subprocess
import webbrowser
import sys

def generate_documentation():
    print("🚀 Generating MemForge Documentation...")
    
    # Get the project root directory (one level up from scripts folder)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    print(f"📁 Script directory: {script_dir}")
    print(f"📁 Project root: {project_root}")
    
    # Change to project root directory
    os.chdir(project_root)
    print(f"📁 Working directory: {os.getcwd()}")
    
    # Check if Doxygen is available
    try:
        subprocess.run(["doxygen", "--version"], check=True, capture_output=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ ERROR: Doxygen not found!")
        print("Please install Doxygen from: https://www.doxygen.nl/download.html")
        input("Press Enter to exit...")
        return False
    
    # Check if Doxyfile exists in project root
    doxyfile_path = os.path.join(project_root, "Doxyfile")
    if not os.path.exists(doxyfile_path):
        print("❌ ERROR: Doxyfile not found in project root!")
        print(f"Expected at: {doxyfile_path}")
        input("Press Enter to exit...")
        return False
    
    # Generate documentation
    print("📝 Running Doxygen...")
    result = subprocess.run(["doxygen", "Doxyfile"], capture_output=True, text=True, cwd=project_root)
    
    if result.returncode != 0:
        print("❌ ERROR: Documentation generation failed!")
        if result.stderr:
            print("Errors:", result.stderr)
        input("Press Enter to exit...")
        return False
    
    # Check if index.html was created in the correct location
    # Based on Doxyfile: OUTPUT_DIRECTORY = docs/doxygen, HTML_OUTPUT = html
    docs_path = os.path.join(project_root, "docs", "html", "index.html")
    
    print(f"🔍 Looking for documentation at: {docs_path}")
    
    if os.path.exists(docs_path):
        abs_path = os.path.abspath(docs_path)
        print(f"✅ SUCCESS: Documentation generated at {abs_path}")
        
        # Ask to open in browser
        response = input("\nOpen in browser now? (Y/n): ").strip().lower()
        if response in ['', 'y', 'yes']:
            # Convert Windows path to file URL
            if os.name == 'nt':  # Windows
                file_url = f"file:///{abs_path.replace(os.sep, '/')}"
            else:  # Linux/Mac
                file_url = f"file://{abs_path}"
            
            print(f"📖 Opening documentation in browser...")
            webbrowser.open(file_url)
    else:
        print(f"❌ ERROR: index.html not found at expected location!")
        print(f"Expected: {docs_path}")
        
        # Debug: Check what files exist
        print("\n🔍 Checking project structure...")
        docs_dir = os.path.join(project_root, "docs")
        if os.path.exists(docs_dir):
            print("Contents of docs/ directory:")
            for item in os.listdir(docs_dir):
                item_path = os.path.join(docs_dir, item)
                if os.path.isdir(item_path):
                    print(f"  📁 {item}/")
                    # Show contents of subdirectories
                    for subitem in os.listdir(item_path):
                        print(f"    📄 {subitem}")
                else:
                    print(f"  📄 {item}")
        else:
            print("docs/ directory not found!")
        
        return False
    
    return True

if __name__ == "__main__":
    generate_documentation()
    input("Press Enter to exit...")