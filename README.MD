
Freaky Engine (The freakiest engine for the freakiest people 😉)

https://github.com/CITM-UPC/FreakyEngine_Group5


List of team members

Alex Becerra https://github.com/Bonnie041008
Samuel Martinez https://github.com/Samu163
Oliver Santos https://github.com/Esnaiper

How to use the engine

Right click - Rotate the camera 
Right click + AWSD - Fps camera movement
Right click + Q - Up the camera
Right Click + E: Move the camera down.
Right Click + Shift + WASD: Fast FPS camera movement.
Right Click + Alt: Orbit around the selected object.
F: Focus on the selected object.
I: Spawn a triangle at the mouse position.
O: Spawn a square at the mouse position.
P: Spawn a cube at the mouse position.
Delete (SUPR): Delete the selected game object.
Left Click: Select an object in the scene.


Inspector

You can view all the properties of the selected game object (transform, mesh, texture).

-Transform

You can modify the values for Position, Rotation, and Scale by dragging or double-clicking the values.

-Mesh and Texture

You can see the file path for both assets.

Hierarchy

You can parent objects, but this creates a duplicate of the child object. To fix this, delete the original duplicate, leaving only the child and the parent.
It is not possible to re-parent game objects.
You can create empty game objects by clicking the menu Mesh -> Spawn Empty gameObject.
When creating a new game object while another object is selected, the new object will automatically be parented to the selected object.

Asset Folder

You can view the files in the Assets folder on your computer from this menu.
You can move files to different folders and delete them as well.

Custom File Format

When you import a model, a .freak file will be automatically generated inside the folder Assets/Library/CustomFreaks.
When you import a texture, a .texture file will be automatically generated inside the folder Assets/Library/CustomTextures.

Scene Serialization

You can save the scene from the menu Scene -> Save Scene.This will save all game objects and their components to a .scene file. (We recommend saving it in Assets/Library/Scenes.)
Custom textures are not properly loaded, but meshes will be visible.
The release includes the bakerhouse scene. However, the street scene can also be loaded, but game objects will be rotated 90 degrees on the Y-axis.
To test it, uncomment line 347 in Main.cpp://GameObject scene3 = SceneImporter::loadFromFile("Assets/street2.fbx");

Frustum Culling

The camera Test Camera projects four planes that handle rendering all the game objects within them.
Modifying the transformations of the Test Camera will affect the frustum planes.

Notes

The bakerhouse is not in the resource folder and must be correctly initialized.
