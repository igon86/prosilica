function z = valutaPuntoPotato(A,x_0,y_0,sigma_x,sigma_y,a,b,c,B,s_x,s_y,x,y)
    %this function just evaluate the 2D gaussian with slope and POTATO EFFECT in (x,y)
    z = (A*(1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2)))*(1+B*sin(s_x*x + s_y*y)) + a*x + b*y +c;
end