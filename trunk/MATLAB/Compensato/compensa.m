function [ compensated ] = compensa( image , alpha )
%COMPENSA FUNCTION THAT COMPENSATE FOR THE CAMERA NON LINEARITY
%   every pixel (x,y) is multiplied by a factor 1 + alpha*z where z is the
%   luminosity of the pixel

    imag = double(image);
    img = imag(:);
    m = size(img);
    [dimy,dimx] = size(image);
    temp = zeros(m,1);
    
    for i=1:m
        temp(i) = img(i)*(1 + alpha*img(i) );
    end
    
    compensated = reshape(temp,dimy,dimx);
end

