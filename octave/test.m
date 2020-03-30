A(:,:,1) = [1 2 3; 4 5 6; 7 8 9]
A(:,:,2) = [10 20 30; 40 50 60; 70 80 90]
A(:,:,3) = [11 22 33; 44 55 66; 77 88 99]
A = uint8(A)

A1(:,:,1) = [1 2 3; 4 5 6; 7 8 9]
A1(:,:,2) = A1(:,:,1)
A1(:,:,3) = A1(:,:,1)
A1 = uint8(A1)

H = fspecial('prewitt');
[frows, fcols, tmp] = size(H);
##
# B = padarray (A, floor([4,4]), "symmetric")
##C = conv2(B, H, "full")

I = imread('../cmake-build-debug/img.png');
##subplot(2,2,1);imshow(I);title('Original Image'); 
##H = fspecial('average', [3, 3]);
BMP = imfilter(I,H,'both', 0, 'full', 'conv');
imwrite(BMP, 'img.bmp')
subplot(2,2,2);imshow(BMP);title('Motion Blurred Image');
##H = fspecial('disk',10);
##blurred = imfilter(I,H,'replicate');
##subplot(2,2,3);imshow(blurred);title('Blurred Image');
##H = ('unsharp');
##sharpened = imfilter(I,H,'replicate');
##subplot(2,2,4);imshow(sharpened);title('Sharpened Image');