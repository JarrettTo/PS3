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
import java.util.List;


class DrawPanel extends JPanel {
    private int peripheryWidth = 33;
    private int peripheryHeight = 19;

    protected void paintComponent(Graphics g, boolean mode,Sprite sprite, List<Particle> particles, AtomicInteger actualFramesDrawn, int fps, List<Wall> walls) {
        super.paintComponent(g);
        g.setColor(Color.BLACK);
        if(mode){
            explorer(g, sprite, particles, actualFramesDrawn,fps);
        }else{
            developer(g, particles, walls, sprite, fps, actualFramesDrawn);
        }

    }

    private void explorer(Graphics g, Sprite sprite, List<Particle> particles, AtomicInteger actualFramesDrawn, int fps) {
        int canvasWidth = this.getWidth();
        int canvasHeight = this.getHeight();
        int spriteCenterX = sprite.getX();
        int spriteCenterY = sprite.getY();

        int leftBound = spriteCenterX - peripheryWidth / 2;
        int rightBound = spriteCenterX + peripheryWidth / 2;
        int topBound = spriteCenterY + peripheryHeight / 2;
        int bottomBound = spriteCenterY - peripheryHeight / 2;

        double scaleX = (double) canvasWidth / peripheryWidth;
        double scaleY = (double) canvasHeight / peripheryHeight;

        sprite.draw(g, this);

        for (Particle particle : particles) {
            if (particle.getX()+5 >= leftBound && particle.getX() -5<= rightBound &&
                particle.getY()+5 >= bottomBound && particle.getY()-5 <= topBound) {
                    int relativeX = (int) particle.getX() - leftBound;
                    int relativeY = topBound - (int) particle.getY();

                    int scaledParticleX = (int) (relativeX * scaleX);
                    int scaledParticleY = (int) (relativeY * scaleY);
                    particle.drawScaled(this, g, scaledParticleX, scaledParticleY);
            }
        }

        
        g.drawString("FPS: " + fps, 10, 20);
        actualFramesDrawn.incrementAndGet();
    }



    private void developer(Graphics g , List<Particle> particles, List<Wall> walls, Sprite sprite, int fps, AtomicInteger actualFramesDrawn){
        for (Particle p : particles) {
            p.draw(this,g);
        }
        walls.parallelStream().forEach(w -> w.draw(g, this));
        if(sprite != null){
            sprite.draw(g, this);
        }

        g.drawString("FPS: " + fps, 10, 20);
        actualFramesDrawn.incrementAndGet();
    }
}