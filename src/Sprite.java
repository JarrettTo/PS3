import java.awt.*;
import javax.swing.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.net.URL;

import javax.imageio.ImageIO;

public class Sprite {
    private int x;
    private int y;
    private BufferedImage[] images; // Array to hold multiple sprite images
    private int currentImageIndex = 0; // Index of the current image

    public Sprite(int x, int y) {
        this.x = x;
        this.y = y;
        loadImages(); // Load multiple images
    }

    private void loadImages() {
        try {
            images = new BufferedImage[3];
            // Using File objects with absolute paths derived from a relative path
            images[0] = ImageIO.read(new File("../resources/pingu.png").getCanonicalFile());
            images[1] = ImageIO.read(new File("../resources/pingu2.png").getCanonicalFile());
            images[2] = images[0]; // Reuse the first image for standing still if appropriate
        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("Error loading images. Make sure the path is correct.");
        }
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
    public void setX(int x) {
        this.x = x;
    }

    public void setY(int y) {
        this.y = y;
    }


    public void draw(Graphics g, JPanel drawPanel) {
        int canvasWidth = drawPanel.getWidth();
        int canvasHeight = drawPanel.getHeight();

        double scaleX = (double) canvasWidth / images[currentImageIndex].getWidth();
        double scaleY = (double) canvasHeight / images[currentImageIndex].getHeight();

        double scale = Math.min(scaleX, scaleY);

        int scaledWidth = (int) (images[currentImageIndex].getWidth() * scale);
        int scaledHeight = (int) (images[currentImageIndex].getHeight() * scale);
        int centeredX = (canvasWidth / 2) - (scaledWidth / 2);
        int centeredY = (canvasHeight / 2) - (scaledHeight / 2);

        g.drawImage(images[currentImageIndex], centeredX, centeredY, scaledWidth, scaledHeight, null);
    }
    public void drawSquare(Graphics g, JPanel drawPanel) {
        // Set the color to red
        g.setColor(Color.RED);
    
        // Draw a filled rectangle (square)
        // The "- 10" part is to center the square around the (x, y) coordinates
        g.fillRect((int) Math.round(x) - 5, (int) Math.round(drawPanel.getHeight() - y) - 5, 10, 10);
    }
    public void drawScaledSquare(Graphics g, JPanel drawPanel) {
        // Set the color to red
        g.setColor(Color.RED);
    
        // Set the square size as a fraction of the smallest panel dimension
        int squareSize = Math.min(drawPanel.getWidth(), drawPanel.getHeight()) / 4; // for example, 1/4th the size of the smaller dimension
        
        // Calculate the top-left corner (x, y) coordinates to center the square
        int x = (drawPanel.getWidth() - squareSize) / 2;
        int y = (drawPanel.getHeight() - squareSize) / 2;
        
        // Draw a filled square centered on the panel
        g.fillRect(x, y, squareSize, squareSize);
    }
    public void drawScaledSquareOtherSprites(Graphics g, int scaledX, int scaledY, JPanel drawPanel) {
        // Set the color for other sprites. You can use different colors to differentiate.
        g.setColor(Color.RED);
        
        // Set a fixed square size. Adjust this value as needed.
        int squareSize = Math.min(drawPanel.getWidth(), drawPanel.getHeight()) / 4; // for example, 1/4th the size of the smaller dimension
    
        // Draw a filled square at the given scaled (x, y) coordinates
        g.fillRect(scaledX - squareSize / 2, scaledY - squareSize / 2, squareSize, squareSize);
    }
}
    
    
