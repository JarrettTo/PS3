import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Optional;
import java.util.concurrent.ForkJoinPool;

import javax.swing.SwingUtilities;
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

public class ServerSimulator {
    private int port;
    private final ForkJoinPool pool;
    private AtomicInteger actualFramesDrawn = new AtomicInteger();
    private int fps = 0;
    private long lastUpdateTime;
    private Sprite sprite;
    public ServerSimulator(int port) {
        pool = new ForkJoinPool();
        
        this.port = port;
    }

    public void startServer() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server started on port " + port);

            while (true) {
                Socket clientSocket = serverSocket.accept();    
                System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());

                // Handle client in a separate thread
                new ClientHandler(clientSocket).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public int getAndResetFrameCount() {
        return actualFramesDrawn.getAndSet(0);
    }
    



    private void startSimulationThread() {
        Runnable simulationTask = () -> {
            while (!Thread.currentThread().isInterrupted()) {
                long now = System.nanoTime();
                double deltaTime = (now - lastUpdateTime) / 1_000_000_000.0;
                lastUpdateTime = now;
                


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
   

    private static class ClientHandler extends Thread {
        private Socket clientSocket;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }

        @Override
        public void run() {
            // Implement communication with client
            System.out.println("Handling client communication");
        }
    }

    public static void main(String[] args) {
        ServerSimulator server = new ServerSimulator(12345);
        server.startServer();
    }
}
