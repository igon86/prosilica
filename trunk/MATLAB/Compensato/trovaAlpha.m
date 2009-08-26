function [ R2 ] = trovaAlpha( image )
%TROVAALPHA Summary of this function goes here
%   Detailed explanation goes here
    
    range = 22;

    alpha = zeros(range,1);
    alpha(1) = 0;
    alpha(2) = 10^-6;
    
    
    R2 = zeros(range,1);
    R2(1) = airyFastFit(image,194,4,200.5,250.5,0.001,0.001,0);
    
    for i=2:range
        %image
        fprintf(1,'iteration %d with alpha: %d\n',i,alpha(i));
        toDo = compensa(image,alpha(i));
        
        %related error
        R2(i) = airyFastFit(toDo,(1+alpha(i)*194)*194,4,200.5,250.5,0.001,0.001,0);
        
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
end

