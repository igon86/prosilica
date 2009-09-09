function z = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,B,alpha,beta,x,y,z)
    %this function just evaluate the 2D gaussian in (x,y)
    if z <= 10
    z= (A*exp(-((x-x_0)^2/sigma_x^2 + (y-y_0)^2/sigma_y^2))) +a*x +b*y + c;
    else
    z= (A*exp(-((x-x_0)^2/sigma_x^2 + (y-y_0)^2/sigma_y^2))) +a*x +b*y + c + B*log(z)*sin(alpha*x+beta*y);
    end
end