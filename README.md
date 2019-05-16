# VTK_4D-CT

## Keyboard Controls
1. Z = Start animation
2. X = Stop animation
3. Down Arrow = increase timer duration (slow down animation)
4. Up Arrow = decrease timer duration (speed up animation)
5. Right mouse click = select point on the volume's surface
6. Left mouse (click and drag) = pan camera
7. Mouse scroll wheel = zoom in/out
8. D = draw lines between points and calculate distances
9. A = calculate angles and draw angles
10. R = reset point picking, lines, angles, and distances
11. N = print current frame information to terminal
12. M = print information on all frames to CSV file

## How To Run
1. Create a folder for the build (e.g. bin, build, etc.)
2. Build with CMake and your favorite compiler.
3. Run the executable that is generated in the bin\Debug folder from the command line
   
    ```
    vtk4DCT.exe
    ```

4. Note that paths to directories or files has been hardcoded!
