# CGproject
This is the final project of the course Computer Graphics.
In this project, we use [Assimp](https://github.com/assimp/assimp) library to help us read and build the 3D model. The lighting method is a kind of physical base rendering -- [Cook-Torrance](http://www.cs.columbia.edu/~belhumeur/courses/appearance/cook-torrance.pdf). To make the scenes more reality, we implement the DoF & Bokeh effect with CoC in the 3D model base on the paper [Efficiently Simulating the Bokeh of Polygonal Apertures in a Post-Process Depth of Field Shader](http://ivizlab.sfu.ca/papers/cgf2012.pdf).



## Compile & Execute

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


## How to use
In our project, there are several functional keys can control the parameters.
######[W]/[w]	move forward
######[S]/[s]	move backward
######[A]/[a]	move to the left
######[D]/[d]	move to the right
######[M]/[m]	able / disable the mouse
######[H]/[h]	able / disable the HDR mode
######[C]/[c]	able / disable the CoC map	
######[B]/[b]	able / disable the blur	
######[+]		zoom in
######[-]		zoom out
######[.]		make the focus distance larger
######[,]		make the focus distance smaller
######[↑]		make the focus lens larger
######[↓]		make the focus lens smaller
######[→]		make the aperture size larger
######[←]		make the aperture size smaller
######[1]		change the mouse moving coordination to x axis
######[2]		change the mouse moving coordination to y axis
######[3]		change the mouse moving coordination to z axis
######[4]		change the aperture shape to square
######[5]		change the aperture shape to diamond
######[6]		change the aperture shape to parallelogram
######[7]		change the aperture shape to stellate
######[9]		use the lights for bedroom model
######[0]		use the lights for mini market model



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
