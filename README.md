# Colorizeitor
Trabalho 2 para a disciplina de programação de baixo nível

## ToDo
- [x] Grayscale
- [x] Region identification
- [x] Region coloring
- [x] Color same shades of gray of same color
- [ ] Better identify black border (harden lines, more contrast on image, less possible gray tones)
- [ ] Optimize region identification
- [ ] Optimizations

## Sources

https://en.wikipedia.org/wiki/Image_segmentation
https://en.wikipedia.org/wiki/Region_growing

### Color comparsion
- http://www.brucelindbloom.com/index.html?Eqn_DeltaE_CIE76.html
- http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
- http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
- https://docs.opencv.org/2.4/modules/imgproc/doc/miscellaneous_transformations.html?highlight=cvtcolor#cvtcolor
- https://www.cs.rit.edu/~ncs/color/t_convert.html#RGB%20to%20XYZ%20&%20XYZ%20to%20RGB
- https://en.wikipedia.org/wiki/CIELAB_color_space#Converting_between_CIELAB_and_CIE_XYZ_coordinates
- https://en.wikipedia.org/wiki/Color_difference#CIE76

## Explanation

- transforma a img em escala de cinza
- usa region growning para identificar as regioes!
- cria um "dicionario" que associa uma cor a um tom de cinza
- eh feito um sharding das cores no montante total de possiveis cores
