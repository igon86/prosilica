function z = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y)
    r = (  ( sigma_x*((x-x_0)^2) ) + ( sigma_y * ((y-y_0)^2) ) )^0.5;
    %this function just evaluate the 2D gaussian in (x,y) 
    z = A*(   ((  sin(r)   )^2) / ((r)^2 )  ) +a*x +b*y + c;
end