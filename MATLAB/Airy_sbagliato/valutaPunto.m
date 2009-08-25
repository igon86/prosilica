function z = valutaPunto(A,x_0,y_0,varianza,a,b,c,x,y)
    r = (((x-x_0)^2) + ((y-y_0)^2))^0.5;
    %this function just evaluate the 2D gaussian in (x,y) 
    z= A*( sin(varianza*r) / ( varianza*r) ) +a*x +b*y + c;
end