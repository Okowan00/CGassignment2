# CGAssignment2

## How to Run (Visual Studio)

1. **Download and unzip** the project from GitHub.  
2. **Open** `CGAssignment2.sln` in Visual Studio.  
3. **Set build config** to `Debug` and `x86`.  
4. **Press F5** to build and run the specific problem (Q1, Q2, or Q3).

Each problem is a separate project:
- `Q1` : Basic Phong Shading
- `Q2` : Gamma Correction (γ = 2.2)
- `Q3` : Antialiasing (64 samples per pixel with box filter)

💡 You can right-click on the desired project (e.g., `Q2`) in the Solution Explorer and choose **"Set as Startup Project"** before running.

---

## Required Settings

- **Include Directory**  
  `Project → Properties → C/C++ → General → Additional Include Directories`  
  → `$(ProjectDir)include`

- **Library Directory**  
  `Project → Properties → Linker → General → Additional Library Directories`  
  → `$(ProjectDir)lib`

- **Linker Input**  
  `Project → Properties → Linker → Input → Additional Dependencies`  
  → `freeglut.lib`

✅ No external installation is required.  
✅ All needed libraries are already included in the project folder.

---

## Screenshot Location

📸 Output screenshots are saved in two places:
- Inside each project folder: `Q1/`, `Q2/`, `Q3/`
- Also in the root project folder: `CGAssignment2/`

Make sure to check both for verification.
