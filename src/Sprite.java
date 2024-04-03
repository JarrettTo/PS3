import java.awt.*;
import javax.swing.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

public class Sprite {
    private int x;
    private int y;
    private BufferedImage[] images; // Array to hold multiple sprite images
    private int currentImageIndex = 0; // Index of the current image

    public Sprite(int x, int y) {
        this.x = x;
        this.y = y;
    
    }

    

    public void move(int dx, int dy) {
        x += dx;
        y += dy;
        if (dx != 0 || dy != 0) { // If moving
            if (currentImageIndex == 0) {
                currentImageIndex = 1; // Change to pingu2 image
            } else {
                currentImageIndex = 0; // Change to pingu image
            }
        } else {
            currentImageIndex = 2; // Change to pingu image when standing still
        }
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    
}
