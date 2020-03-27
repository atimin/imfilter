A = [1 2 3; 4 5 6; 7 8 9];
B = padarray (A, [5 5], "symmetric", "both")


##I = imread('img.png');
##subplot(2,2,1);imshow(I);title('Original Image'); 
##H = fspecial('average', [3, 3]);
##MotionBlur = imfilter(I,H,'replicate');
##subplot(2,2,2);imshow(MotionBlur);title('Motion Blurred Image');
##H = fspecial('disk',10);
##blurred = imfilter(I,H,'replicate');
##subplot(2,2,3);imshow(blurred);title('Blurred Image');
##H = ('unsharp');
##sharpened = imfilter(I,H,'replicate');
##subplot(2,2,4);imshow(sharpened);title('Sharpened Image');