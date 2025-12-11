using System;
using System.IO.Ports;
using System.Windows.Forms;

namespace WinFormsApp1
{
    public partial class Form1 : Form
    {
        private SerialPort bluetoothPort; // Pour gérer la connexion Bluetooth

        public Form1()
        {
            InitializeComponent();
        }

        // Chargement du formulaire
        private void Form1_Load(object sender, EventArgs e)
        {
            try
            {
                // Chemin relatif de l'image
                string imagePath = Path.Combine(Application.StartupPath, "voiture.jpg");

                // Charger l'image dans le PictureBox
                pictureBox1.Image = Image.FromFile("C:\\Users\\USER\\source\\repos\\WinFormsApp1");

                // Ajuster la taille de l'image dans le PictureBox
                pictureBox1.SizeMode = PictureBoxSizeMode.StretchImage;

                label1.Text = "Image chargée avec succès.";
            }
            catch (Exception ex)
            {
                label1.Text = $"Erreur lors du chargement de l'image : {ex.Message}";
            }

            label2.Text = $"Erreur lors de la réception des données";
            label3.Text = $"NO_DATA";
            label4.Text = $"NO_DATA";

            // Récupérer et afficher les ports COM disponibles
            string[] ports = SerialPort.GetPortNames();
            comboBox1.Items.AddRange(ports);

            if (ports.Length > 0)
            {
                comboBox1.SelectedIndex = 0; // Sélectionner le premier port par défaut
                label1.Text = "Ports disponibles détectés.";
            }
            else
            {
                label1.Text = "Aucun port COM détecté.";
            }

            // Initialiser les barres de progression
            progressBar1.Minimum = 0;
            progressBar1.Maximum = 100;
            progressBar1.Value = 0;

            progressBar2.Minimum = 0;
            progressBar2.Maximum = 100;
            progressBar2.Value = 0;
        }

        // Bouton "Connecter"
        private void button1_Click(object sender, EventArgs e)
        {
            if (comboBox1.SelectedItem != null)
            {
                try
                {
                    // Initialiser et ouvrir la connexion Bluetooth
                    bluetoothPort = new SerialPort(comboBox1.SelectedItem.ToString(), 9600); // Baudrate = 9600
                    bluetoothPort.Open();

                    // Abonner à l'événement DataReceived pour afficher les messages reçus
                    bluetoothPort.DataReceived += BluetoothPort_DataReceived;

                    label1.Text = "Connexion Bluetooth établie !";
                }
                catch (Exception ex)
                {
                    label1.Text = $"Erreur : {ex.Message}";
                }
            }
            else
            {
                label1.Text = "Veuillez sélectionner un port COM.";
            }
        }

        // Bouton "Avancer"
        private void button3_Click(object sender, EventArgs e)
        {
            if (bluetoothPort != null && bluetoothPort.IsOpen)
            {
                bluetoothPort.Write("F"); // Envoyer la commande pour avancer
                label1.Text = "Commande 'Avancer' envoyée.";
            }
            else
            {
                label1.Text = "Aucune connexion Bluetooth n'est active.";
            }
        }

        // Bouton "Tourner Droite"
        private void button4_Click(object sender, EventArgs e)
        {
            if (bluetoothPort != null && bluetoothPort.IsOpen)
            {
                bluetoothPort.Write("R"); // Envoyer la commande pour tourner à droite
                label1.Text = "Commande 'Tourner Droite' envoyée.";
            }
            else
            {
                label1.Text = "Aucune connexion Bluetooth n'est active.";
            }
        }

        // Bouton "Tourner Gauche"
        private void button5_Click(object sender, EventArgs e)
        {
            if (bluetoothPort != null && bluetoothPort.IsOpen)
            {
                bluetoothPort.Write("L"); // Envoyer la commande pour tourner à gauche
                label1.Text = "Commande 'Tourner Gauche' envoyée.";
            }
            else
            {
                label1.Text = "Aucune connexion Bluetooth n'est active.";
            }
        }

        // Bouton "Reculer"
        private void button6_Click(object sender, EventArgs e)
        {
            if (bluetoothPort != null && bluetoothPort.IsOpen)
            {
                bluetoothPort.Write("B"); // Envoyer la commande pour reculer
                label1.Text = "Commande 'Reculer' envoyée.";
            }
            else
            {
                label1.Text = "Aucune connexion Bluetooth n'est active.";
            }
        }

        // Méthode pour gérer les données reçues du robot
        private void BluetoothPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string data = bluetoothPort.ReadLine(); // Lire une ligne reçue
                Invoke(new Action(() =>
                {
                    label2.Text = $"Données reçues";

                    if (data.StartsWith("F"))
                    {
                        label3.Text = $"Données reçues : {data}";
                        int frontDistance = int.Parse(data.Replace("F:", ""));
                        progressBar1.Value = Math.Clamp(frontDistance, progressBar1.Minimum, progressBar1.Maximum);
                    }
                    else if (data.StartsWith("B"))
                    {
                        label4.Text = $"Données reçues : {data}";
                        int backDistance = int.Parse(data.Replace("B:", ""));
                        progressBar2.Value = Math.Clamp(backDistance, progressBar2.Minimum, progressBar2.Maximum);
                    }
                }));
            }
            catch (Exception ex)
            {
                Invoke(new Action(() =>
                {
                    label2.Text = $"Erreur lors de la réception des données : {ex.Message}";
                    label3.Text = $"NO_DATA";
                    label4.Text = $"NO_DATA";
                }));
            }
        }

        // ComboBox pour sélectionner les ports COM
        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            // Met à jour le label pour indiquer le port sélectionné
            label1.Text = $"Port sélectionné : {comboBox1.SelectedItem}";
        }
    }
}
