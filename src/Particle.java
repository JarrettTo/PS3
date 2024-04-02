import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.Optional;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;
import java.util.concurrent.atomic.AtomicInteger;

class Particle {
    double x, y;
    double angle, velocity;

    public Particle(double x, double y, double angle, double velocity) {
        this.x = x;
        this.y = y;
        this.angle = Math.toRadians(angle);
        this.velocity = velocity;
    }
    public void drawScaled(DrawPanel drawPanel, Graphics g, int scaledX, int scaledY) {
        int canvasWidth = drawPanel.getWidth(); // 1280
        int canvasHeight = drawPanel.getHeight(); // 720
    
        // Periphery dimensions
        final int peripheryWidth = 33;
        final int peripheryHeight = 19;

        double scaleX = (double) canvasWidth / peripheryWidth;
        double scaleY = (double) canvasHeight / peripheryHeight;

        double scale = Math.min(scaleX, scaleY);
        int spriteWidth = 5; 
        int spriteHeight = 5; 
        int scaledWidth = (int) (spriteWidth * scale);
        int scaledHeight = (int) (spriteHeight * scale);
        g.setColor(Color.BLACK); 
        g.fillOval(scaledX, scaledY, scaledWidth, scaledHeight);
    }
    public void move(double deltaTime) {
        x += velocity * Math.cos(angle) * deltaTime;
        y += velocity * Math.sin(angle) * deltaTime;
    }
    public double getX(){
        return x;
    }
    public double getY(){
        return y;
    }
    public void checkCollisionWithBounds(int width, int height) {
        if (x < 0 || x > width) {
            angle = Math.PI - angle;
        }

        if (y < 0 || y > height) {
            angle = -angle;
        }
    }
    
    public Point2D.Double getNextPosition(double deltaTime) {
        double newX = x + velocity * Math.cos(angle) * deltaTime;
        double newY = y + velocity * Math.sin(angle) * deltaTime;
        return new Point2D.Double(newX, newY);
    }

    public void handleCollision(Point2D.Double collisionPoint, Wall wall) {
        this.x = collisionPoint.x;
        this.y = collisionPoint.y;
        double wallAngle = Math.atan2(wall.endY - wall.startY, wall.endX - wall.startX);
        double incidenceAngle = angle - wallAngle;
        angle = wallAngle - incidenceAngle;
        x += 0.1 * Math.cos(angle);
        y += 0.1 * Math.sin(angle);
    }

    public void draw(DrawPanel drawPanel,Graphics g) {
        g.fillOval((int) Math.round(x), (int) Math.round(drawPanel.getHeight() - y), 10, 10);
    }
}
