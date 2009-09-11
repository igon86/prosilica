function derivateg()
    syms y x z x_0 y_0 sigma_x sigma_y A a b c;
    
    z = A*(1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2)) + a*x + b*y +c;
    
    diff(z,A)
    diff(z,x_0)
    diff(z,y_0)
    diff(z,sigma_x)
    diff(z,sigma_y)
    diff(z,a)
    diff(z,b)
    diff(z,c)
end