for file in ./ref/*.png; do 
    convert $file -compress none "`basename $file .png`.ppm";
    mv `basename $file .png`.ppm ./ppm/;
done