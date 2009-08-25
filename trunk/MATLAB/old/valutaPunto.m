function z = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,x,y)
    %estraggo le coordinate dal punto
    z=A*exp(-((x-x_0)^2/sigma_x^2 + (y-y_0)^2/sigma_y^2));
end