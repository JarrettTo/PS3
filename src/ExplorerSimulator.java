import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.UUID;

public class ExplorerSimulator extends JFrame {
    private ArrayList<Particle> particles;
    private final ArrayList<Wall> walls;
    private final DrawPanel drawPanel;
    private final JTextField startField;
    private final JTextField endField;
    private final JTextField startVelocityField;
    private final JTextField endVelocityField;
    private final JTextField startAngleField;
    private final JTextField endAngleField;
    private final JTextField numParticlesField;
    private final JTextField startWallField;
    private final JTextField endWallField;
    private final String clientId;
    private AtomicInteger actualFramesDrawn = new AtomicInteger();
    private long lastTime = System.nanoTime();
    private final double nsPerTick = 1000000000D / 60D;
    private int frames = 0;
    private int fps = 0;
    private long lastUpdateTime;
    private ConcurrentHashMap<String, Sprite> clientSprites = new ConcurrentHashMap<>();
    private final ForkJoinPool pool;
    private long lastHeartbeatTime = System.currentTimeMillis();
    private Sprite sprite;
    private boolean explorer = false;
    public ExplorerSimulator() {
        super("Particle Simulator");
        pool = new ForkJoinPool();
        lastUpdateTime = System.nanoTime();
        particles = new ArrayList<>();
        walls = new ArrayList<>();
        drawPanel = new DrawPanel();
        drawPanel.setPreferredSize(new Dimension(1280, 720));
        add(drawPanel, BorderLayout.CENTER);
        pack();
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setVisible(true);
        startSimulationThread();
        startGameLoop();

        JPanel controlPanel = new JPanel();
        controlPanel.setLayout(new BorderLayout());
        JPanel inputPanel = new JPanel();
        inputPanel.setLayout(new GridLayout(20, 2));
        startWallField = new JTextField(10);
        endWallField = new JTextField(10);
        startField = new JTextField(10);
        endField = new JTextField(10);
        startVelocityField = new JTextField(10);
        startAngleField = new JTextField(10);
        numParticlesField = new JTextField(10);
        endAngleField = new JTextField(10);
        endVelocityField = new JTextField(10);
        clientId = UUID.randomUUID().toString();

        System.out.println("Explorer Mode");
        explorer = true;
        showModeCombobox(inputPanel);
        sprite = new Sprite(100, 100); 
        setupKeyListener();
        startUdpClient();
        sendSpritePosition(sprite.getX(), sprite.getY());
        this.addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                sendDisconnectMessage();
                System.exit(0);
            }
        });
        listenForHeartbeat();
        startHeartbeatChecker();
    }
    private void showModeCombobox(JPanel inputPanel){
        inputPanel.add(new JLabel(" Mode:"));

    }
    private void startHeartbeatChecker() {
        new Thread(() -> {
            while (true) {
                long currentTime = System.currentTimeMillis();
                if (currentTime - lastHeartbeatTime > 30000) { // 30 seconds without heartbeat
                    JOptionPane.showMessageDialog(null, "Lost connection to the server. Closing.");
                    System.exit(0); // Close the client
                }
                try {
                    Thread.sleep(5000); // Check every 5 seconds
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
        }).start();
    }
    
    private void listenForHeartbeat() {
        Thread listenerThread = new Thread(() -> {
            try (MulticastSocket socket = new MulticastSocket(4449)) { // Use the appropriate port
                InetAddress group = InetAddress.getByName("230.0.0.1");
                socket.joinGroup(group);
                byte[] buffer = new byte[1024];
                while (true) {
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    String message = new String(packet.getData(), 0, packet.getLength(), StandardCharsets.UTF_8);
                    if (message.contains("\"heartbeat\":true")) {
                        lastHeartbeatTime = System.currentTimeMillis(); // Update last heartbeat time
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        listenerThread.start();
    }
    private void sendDisconnectMessage() {
        try {
            DatagramSocket socket = new DatagramSocket();
            InetAddress serverAddress = InetAddress.getByName("localhost");
            int serverPort = 4445; 
            
            String message = String.format("{\"clientId\":\"%s\",\"disconnect\":true}", clientId);
            byte[] buffer = message.getBytes();
            
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, serverAddress, serverPort);
            socket.send(packet);
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public int getAndResetFrameCount() {
        return actualFramesDrawn.getAndSet(0);
    }
    private void startGameLoop() {
        new Thread(() -> {
            final int targetFPS = 60;
            final long targetTimePerFrame = 1000 / targetFPS;
            long timer = System.currentTimeMillis();
            long lastLoopTime = System.currentTimeMillis();
            while (true) {
                long now = System.currentTimeMillis();
                long elapsedTime = now - lastLoopTime;
                lastLoopTime = now;
                SwingUtilities.invokeLater(drawPanel::repaint);

                if (System.currentTimeMillis() - timer > 500) {
                    fps = getAndResetFrameCount();
                    timer += 1000;
                }
                long sleepTime = targetTimePerFrame - elapsedTime;
                if (sleepTime > 0) {
                    try {
                        Thread.sleep(sleepTime);
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        break;
                    }
                }
            }
        }, "Game Loop Thread").start();
    }

    private void startUdpClient() {
        Thread udpClientThread = new Thread(() -> {
            try (MulticastSocket socket = new MulticastSocket(4446)) {
                InetAddress group = InetAddress.getByName("230.0.0.1");
                socket.joinGroup(group);
                
                byte[] buffer = new byte[1048576];
                while (true) {
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    String receivedJson = new String(packet.getData(), 0, packet.getLength());
                    updateParticlesFromJson(receivedJson);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
        udpClientThread.start();
        Thread udpClientThreadForSprites = new Thread(() -> {
            receiveMulticastData(4447, this::updateSpritesFromJson);
        });
        udpClientThreadForSprites.start();
        listenForShutdownSignal();
    }
    private void listenForShutdownSignal() {
        Thread listenerThread = new Thread(() -> {
            try (MulticastSocket socket = new MulticastSocket(4448)) { 
                InetAddress group = InetAddress.getByName("230.0.0.1");
                socket.joinGroup(group);
                byte[] buffer = new byte[1024];
                while (true) {
                    DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                    socket.receive(packet);
                    String message = new String(packet.getData(), 0, packet.getLength(), StandardCharsets.UTF_8);
                    if (message.contains("\"shutdown\":true")) {

                        System.exit(0);
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        listenerThread.start();
    }

    private void receiveMulticastData(int port, Consumer<String> dataHandler) {
        try (MulticastSocket socket = new MulticastSocket(port)) {
            InetAddress group = InetAddress.getByName("230.0.0.1");
            socket.joinGroup(group);
            byte[] buffer = new byte[1024];
            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);
                String receivedJson = new String(packet.getData(), 0, packet.getLength());
                dataHandler.accept(receivedJson);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    private void updateSpritesFromJson(String json) {
        Set<String> receivedClientIds = new HashSet<>();
        Pattern pattern = Pattern.compile("\\{\"clientId\":\"(.*?)\",\"x\":(\\d+),\"y\":(\\d+)\\}");
        Matcher matcher = pattern.matcher(json);
        System.out.println("DATA:" + json);
        SwingUtilities.invokeLater(() -> {
            while (matcher.find()) {
                String clientId = matcher.group(1);
                int x = Integer.parseInt(matcher.group(2));
                int y = Integer.parseInt(matcher.group(3));
                receivedClientIds.add(clientId); 
              
                if (!clientId.equals(this.clientId)) {
                    clientSprites.compute(clientId, (id, sprite) -> {
                        if (sprite == null) {
                            sprite = new Sprite(x, y);
                        } else {
                            sprite.setX(x); 
                            sprite.setY(y);
                        }
                        return sprite;
                    });
                }
            }

            clientSprites.keySet().removeIf(clientId -> !receivedClientIds.contains(clientId));
            
            drawPanel.repaint();
        });
    }

    private void updateParticlesFromJson(String json) {
        System.out.println("DATA" + json);
        Pattern pattern = Pattern.compile("\\{\"x\":(.*?),\"y\":(.*?),\"velocity\":(.*?),\"angle\":(.*?)\\}");
        Matcher matcher = pattern.matcher(json);
        ArrayList<Particle> updatedParticles = new ArrayList<>();
        
        while (matcher.find()) {
            try {
                double x = Double.parseDouble(matcher.group(1));
                double y = Double.parseDouble(matcher.group(2));
                double velocity = Double.parseDouble(matcher.group(3));
                double angle = Double.parseDouble(matcher.group(4)); 
    
                updatedParticles.add(new Particle(x, y, angle, velocity));
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
        }
        
        SwingUtilities.invokeLater(() -> {
            particles = updatedParticles;
            drawPanel.repaint();
        });
    }

    
    
    
    private void startSimulationThread() {
        Runnable simulationTask = () -> {
            while (!Thread.currentThread().isInterrupted()) {
                long now = System.nanoTime();
                double deltaTime = (now - lastUpdateTime) / 1_000_000_000.0;
                lastUpdateTime = now;
                ParticleUpdateAction updateAction = new ParticleUpdateAction(particles, 0, particles.size(), deltaTime);
                pool.invoke(updateAction);


                SwingUtilities.invokeLater(drawPanel::repaint);

                try {
                    Thread.sleep(16);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            }
        };

        new Thread(simulationTask).start();
    }
    public static Optional<Point2D.Double> findIntersectionPoint(Line2D line1, Line2D line2) {
        double x1 = line1.getX1(), y1 = line1.getY1();
        double x2 = line1.getX2(), y2 = line1.getY2();

        double x3 = line2.getX1(), y3 = line2.getY1();
        double x4 = line2.getX2(), y4 = line2.getY2();

        double den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (den == 0) return Optional.empty();

        double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
        double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            double x = x1 + t * (x2 - x1);
            double y = y1 + t * (y2 - y1);
            return Optional.of(new Point2D.Double(x, y));
        }

        return Optional.empty();
    }

    class DrawPanel extends JPanel {
        private int peripheryWidth = 33;
        private int peripheryHeight = 19;
        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            g.setColor(Color.BLACK);

            if(explorer){
                explorer(g);
            }
            else{
                developer(g);
            }
        }

        private void explorer(Graphics g) {
            if (sprite == null) {
                sprite = new Sprite(100, 100);
  
                return; 
            }
            int canvasWidth = drawPanel.getWidth();
            int canvasHeight = drawPanel.getHeight();
            int spriteCenterX = sprite.getX();
            int spriteCenterY = sprite.getY();

            int leftBound = spriteCenterX - peripheryWidth / 2;
            int rightBound = spriteCenterX + peripheryWidth / 2;
            int topBound = spriteCenterY + peripheryHeight / 2;
            int bottomBound = spriteCenterY - peripheryHeight / 2;

            double scaleX = (double) canvasWidth / peripheryWidth;
            double scaleY = (double) canvasHeight / peripheryHeight;

            sprite.drawScaledSquare(g, drawPanel);

            for (Particle particle : particles) {
                if (particle.getX()+5 >= leftBound && particle.getX() -5<= rightBound &&
                    particle.getY()+5 >= bottomBound && particle.getY()-5 <= topBound) {
                        int relativeX = (int) particle.getX() - leftBound;
                        int relativeY = topBound - (int) particle.getY();

                        int scaledParticleX = (int) (relativeX * scaleX);
                        int scaledParticleY = (int) (relativeY * scaleY);
                        particle.drawScaled(g, scaledParticleX, scaledParticleY);
                }
            }
            drawOtherSprites(g, spriteCenterX, spriteCenterY, scaleX, scaleY); 
            
            g.drawString("FPS: " + fps + " X: "+ spriteCenterX+ " Y: "+spriteCenterY, 10, 20);
            actualFramesDrawn.incrementAndGet();
        }



        private void developer(Graphics g){
            for (Particle p : particles) {
                p.draw(g);
            }
            walls.parallelStream().forEach(w -> w.draw(g));
            if(sprite != null){
                sprite.drawScaledSquare(g, drawPanel);
            }

            g.drawString("FPS: " + fps, 10, 20);
            actualFramesDrawn.incrementAndGet();
        }
        private void drawOtherSprites(Graphics g, int centerX, int centerY, double scaleX, double scaleY) {
            int leftBound = centerX - peripheryWidth / 2;
            int rightBound = centerX + peripheryWidth / 2;
            int topBound = centerY + peripheryHeight / 2;
            int bottomBound = centerY - peripheryHeight / 2;
    
            for (Sprite s : clientSprites.values()) {

                if (s.getX()+5 >= leftBound && s.getX()-5 <= rightBound &&
                    s.getY()+5 >= bottomBound && s.getY()-5 <= topBound) {
                    
                    int relativeX = (int) (scaleX * (s.getX() - leftBound));
                    int relativeY = (int) (scaleY * (topBound - s.getY()));
                    int scaledSpriteX = (int) (relativeX * scaleX);
                    int scaledSpriteY = (int) (relativeY * scaleY);

                    s.drawScaledSquareOtherSprites(g, relativeX, relativeY, this); 
                }
            }
        }
    }
    class ParticleUpdateAction extends RecursiveAction {
        private final ArrayList<Particle> particles;
        private final int start;
        private final int end;
        private final double deltaTime;
        private static final int THRESHOLD = 15;

        ParticleUpdateAction(ArrayList<Particle> particles, int start, int end, double deltaTime) {
            this.particles = particles;
            this.start = start;
            this.end = end;
            this.deltaTime = deltaTime;
        }

        @Override
        protected void compute() {
            if (end - start <= THRESHOLD) {
                for (int i = start; i < end; i++) {
                    Particle particle = particles.get(i);
                    Point2D.Double nextPosition = particle.getNextPosition(deltaTime);
                    Line2D.Double trajectory = new Line2D.Double(particle.x, particle.y, nextPosition.x, nextPosition.y);

                    Point2D.Double closestCollision = null;
                    Wall closestWall = null;
                    double closestDistance = Double.MAX_VALUE;


                    particle.checkCollisionWithBounds(drawPanel);
                    for (Wall wall : walls) {
                        Optional<Point2D.Double> intersectionPoint = findIntersectionPoint(trajectory, wall.toLine2D());
                        if (intersectionPoint.isPresent()) {
                            double distance = intersectionPoint.get().distance(particle.x, particle.y);
                            if (distance < closestDistance) {
                                closestCollision = intersectionPoint.get();
                                closestWall = wall;
                                closestDistance = distance;
                            }
                        }
                    }
                    if (closestCollision != null) {
                        particle.handleCollision(closestCollision, closestWall);
                    } else {

                        particle.move(deltaTime);
                    }
                }
            } else {
                int mid = start + (end - start) / 2;
                ParticleUpdateAction left = new ParticleUpdateAction(particles, start, mid, deltaTime);
                ParticleUpdateAction right = new ParticleUpdateAction(particles, mid, end, deltaTime);
                invokeAll(left, right);
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new ExplorerSimulator().setVisible(true));
    }
    class Particle {
        double x, y;
        double angle, velocity;

        public Particle(double x, double y, double angle, double velocity) {
            this.x = x;
            this.y = y;
            this.angle = Math.toRadians(angle);
            this.velocity = velocity;
        }
        public void drawScaled(Graphics g, int scaledX, int scaledY) {
            int canvasWidth = drawPanel.getWidth(); 
            int canvasHeight = drawPanel.getHeight(); 
        
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
        public void checkCollisionWithBounds(DrawPanel drawPanel) {
            if (x < 0 || x > drawPanel.getWidth()) {
                angle = Math.PI - angle;
            }

            if (y < 0 || y > drawPanel.getHeight()) {
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

        public void draw(Graphics g) {
            g.fillOval((int) Math.round(x), (int) Math.round(drawPanel.getHeight() - y), 10, 10);
        }
    }

    class Wall {
        int startX, startY, endX, endY;

        public Wall(int startX, int startY, int endX, int endY) {
            this.startX = startX;
            this.startY = startY;
            this.endX = endX;
            this.endY = endY;
        }

        public Line2D.Double toLine2D() {
            return new Line2D.Double(startX, startY, endX, endY);
        }

        public void draw(Graphics g) {
            g.drawLine(startX, drawPanel.getHeight() - startY, endX, drawPanel.getHeight() - endY);
        }
    }

    // SPRITE
    public void moveSprite(int dx, int dy) {
        if (sprite != null) {
            int newX = sprite.getX() + dx;
            int newY = sprite.getY() + dy;
            int canvasWidth = drawPanel.getWidth();
            int canvasHeight = drawPanel.getHeight();
            int spriteWidth = sprite.getX();
            int spriteHeight = sprite.getY();

            if (newX >= 0 && newX + spriteWidth <= canvasWidth && newY >= 0 && newY + spriteHeight <= canvasHeight) {
                sprite.move(dx, dy);
                drawPanel.repaint();
            }
            else
                JOptionPane.showMessageDialog(this, "Sprite reached the end of the canvas", "Error", JOptionPane.ERROR_MESSAGE);
            sendSpritePosition(sprite.getX(), sprite.getY());
        }
    }
    private void sendSpritePosition(int x, int y) {
        try {
            DatagramSocket socket = new DatagramSocket();
            InetAddress serverAddress = InetAddress.getByName("localhost");
            int serverPort = 4445; 
            
            String message = String.format("{\"clientId\":\"%s\",\"spriteX\":%d,\"spriteY\":%d}", clientId, x, y);
            byte[] buffer = message.getBytes();
            
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, serverAddress, serverPort);
            socket.send(packet);
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private void setupKeyListener() {
        drawPanel.setFocusable(true);
        drawPanel.requestFocus(); 
        drawPanel.addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                int keyCode = e.getKeyCode();
                if (keyCode == KeyEvent.VK_W || keyCode == KeyEvent.VK_UP) {
                    moveSprite(0, 1);
                } else if (keyCode == KeyEvent.VK_A || keyCode == KeyEvent.VK_LEFT) {
                    moveSprite(-1, 0);
                } else if (keyCode == KeyEvent.VK_S || keyCode == KeyEvent.VK_DOWN) {
                    moveSprite(0, -1);
                } else if (keyCode == KeyEvent.VK_D || keyCode == KeyEvent.VK_RIGHT) {
                    moveSprite(1, 0);
                }
            }
        });
    }
}
