function [ wx,wy,R2 ] = testaCentro( image )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here
    
    range = 10;

    alpha = zeros(range,1);
    alpha(1) = 0;
    alpha(2) = 10^-4;
    
    
    R2 = zeros(range,1);
    wx = zeros(range,1);
    wy = zeros(range,1);
    [R2(1),wx(1),wy(1)] = airyFastFit2(image,194,4,200.5,229.5,0.01,0.01,1);
    
    for i=2:range
        %image
        fprintf(1,'iteration %d with alpha: %d\n',i,alpha(i));
        toDo = compensa(image,alpha(i));
        
        %related error
        [R2(i),wx(i),wy(i)] = airyFastFit2(toDo,(1+alpha(i)*194)*194,4,200.5,229.5,0.01,0.01,1);
        
        %iteration
        if mod(i,2) == 0
            alpha(i+1) = alpha(i)*2;
        else   
            alpha(i+1) = alpha(i)*5;
        end
    end
    alpha
    figure(1);
    semilogx(alpha(1:range),R2(:,1));
    title('Goodness of fit over corrective factor');
    xlabel('alpha');
    ylabel('R2')
    figure(2);
    semilogx(alpha(1:range),wx(:,1));
    title('Goodness of x_0 over corrective factor');
    xlabel('alpha');
    ylabel('x_0_confidence')
    figure(3);
    semilogx(alpha(1:range),wy(:,1));
    title('Goodness of y_0 over corrective factor');
    xlabel('alpha');
    ylabel('y_0_confidence')

end

