function z = valutaPunto(A,b,x_0,y_0,sigma_x,sigma_y,x,y)
    %this function just evaluate the 2D gaussian in (x,y) 
    z= (A*exp(-((x-x_0)^2/sigma_x^2 + (y-y_0)^2/sigma_y^2))) +b;
end