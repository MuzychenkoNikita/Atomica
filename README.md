# 🌌 Atomica  
Atomica is an **interactive atomic simulator** where you can **build an atom from scratch** — placing protons, neutrons, and electrons — and instantly see a **3D ray-marched visualization** of your creation.  
It also features an **AI chat powered by Gemini**, which provides real-time explanations about your custom element.

---

## ⚙️ Features
- 🧩 **2D Constructor:** Drag and attach subatomic particles to form atoms.  
- 🌐 **3D Visualization:** Real-time ray-marched rendering of your atom.  
- 🤖 **AI Integration:** Ask Gemini questions about your created element.  
- 💻 **Cross-Platform:** Works on Windows, macOS, and Linux.  
- 🧠 **All-in-One Setup:** Includes all dependencies and libraries.

---

## 🧰 Requirements
- **C++17 or newer**  
- **Premake5** (included in `Vendor/Binaries/`)  
- **cURL** (required for Gemini chat)  
> All other libraries and includes are already provided in the project.

---

## How to use
Project has support for Windows, MacOS and Linux. All necessary libraries and includes are already within a directory. Scripts for relevant platforms are provided within a `Scripts/` directory to setup project using premake5.
- Windows: open `Scripts/` and run `Setup-Windows.bat`, then open generated Visual Studio solution in root directory `2D-Engine/`
- MacOs: open `Scripts/` with terminal and run `sh Setup-Mac.sh`. If you getting permission errors, you have to give permission for scrpits by typing next two commands `chmod +x ../Vendor/Binaries/Premake/macOS/premake5` and `chmod +x Setup-Mac.sh`. After that Xcode solution will be generated in root directory `2D-Engine/`
- Linux: within 'Scripts/' directory run `chmod +x script.sh && ./script.sh`. Then open generated Gmake solution.

## License
- UNLICENSE for this repository (see `UNLICENSE.txt` for more details)
- Premake is licensed under BSD 3-Clause (see included LICENSE.txt file for more details)
