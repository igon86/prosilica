function b = derivateg(A)
    b=1;
    syms y x z x_0 y_0 sigma_x sigma_y A g;
    
    z = (  ( sigma_x*((x-x_0)^2) ) + ( sigma_y * ((y-y_0)^2) ) )^0.5;
    f = A*(   ((  sin(z)   )^2) / ((z)^2 )  ) ;
    
    diff(f,A)
    diff(f,x_0)
    diff(f,y_0)
    diff(f,sigma_x)
    diff(f,sigma_y)
end