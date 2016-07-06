# CGproject
This is the final project of the course Computer Graphics.
In this project, we use [Assimp](https://github.com/assimp/assimp) library to help us read and build the 3D model. The lighting method is a kind of physical base rendering -- [Cook-Torrance](http://www.cs.columbia.edu/~belhumeur/courses/appearance/cook-torrance.pdf). To make the scenes more reality, we implement the DoF & Bokeh effect with CoC in the 3D model base on the paper [Efficiently Simulating the Bokeh of Polygonal Apertures in a Post-Process Depth of Field Shader](http://ivizlab.sfu.ca/papers/cgf2012.pdf).

## How to use

### Environment setting
Install `glfw`

 -> reference : [CGHW2](https://hackpad.com/CGHW2-IbLuMM0Otih)
 
Install `assimp`

 -> reference : [Assimp](https://github.com/assimp/assimp)
 
Install `SOIL(Simple OpenGL Image Library)`

 -> reference : [SOIL](http://www.lonesock.net/soil.html)
 
### Windows (with MinGW)
`./compile.bat`

`./final_project [OBJ_FILE_PATH]`

* In our project :
*  `./final_project 5337_Interior_Scene_of_Bedroom\slykdrako_quarto01_blender.obj`
*  `./final_project MiniMarket\Shop.obj`

### Windows (with IDE)
You need to install all the libraries mentioned before in your IDE, then you can run our project with IDE.


### Linux / Unix / MacOS
`make`

`./final_project [OBJ_FILE_PATH]`

* In our project : 
*  `./final_project 5337_Interior_Scene_of_Bedroom/slykdrako_quarto01_blender.obj`
*  `./final_project MiniMarket/Shop.obj`


## Demo
### square bokeh
![suqare bokeh](https://raw.githubusercontent.com/WemyJu/CG_final/master/CG_demo_pic/square.png)

### diamond bokeh
![diamond bokeh](https://raw.githubusercontent.com/WemyJu/CG_final/master/CG_demo_pic/diamond.png)

### parallelogram bokeh
![parallelogram bokeh](https://raw.githubusercontent.com/WemyJu/CG_final/master/CG_demo_pic/parallelogram.png)

### stellate bokeh
![stellate bokeh](https://raw.githubusercontent.com/WemyJu/CG_final/master/CG_demo_pic/stellate.png)


## Author
* P76044499 吳敏慈
* H34011314 蘇揚哲
* H34011160 朱宥繐
