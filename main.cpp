#include <ga/GA1DArrayGenome.h>
#include <ga/GASimpleGA.h>
#include <ga/std_stream.h>
#include <ctime>
#include <cstdlib>
#include <numeric>   // Include for std::iota
#include <algorithm> // Include for std::shuffle
#include <random>    // Include for std::mt19937 and std::random_device
#include <unordered_set>
#include <chrono>

using namespace std;

const int BOARD_SIZE = 81;  // 9x9 Sudoku Feld = 81 Einträge für ein 1D Genom
const int POPULATION_SIZE = 820;
const int MAX_GENERATIONS = 2500;
const float MUTATION_RATE = 0.01;
const float CROSSOVER_RATE = 0.9;
const float EXPECTED_FITNESS = 161.0; // Mit unserer Fitnessberechnung muss der Algorithmus einen Fitnessscore von mindestens 161.0 um ein fehlerfreies, Sudokufeld zu generieren.


// Berechnet den Fitness Score
float objective(GAGenome& g) {
    GA1DArrayGenome<int>& genome = (GA1DArrayGenome<int>&)g;
    int fitness = 0;

    // Macht aus dem linearen Genom ein Sudoku Spielfeld
    int board[9][9];
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            board[i][j] = genome.gene(i * 9 + j);
        }
    }

    // Überprüft ob die Spalten des Spielfeldes einzigartig sind (keine Ziffer von 1-9 doppelt vorkommt). Zeilen werden schon richtig initialisiert.
    for (int i = 0; i < 9; ++i) {
        std::unordered_set<int> colCheck;
        for (int j = 0; j < 9; ++j) {
            if (colCheck.insert(board[j][i]).second) {
                fitness++;
            }
        }
    }

    // Überprüft ob die Grids (3x3)) des Spielfeldes einzigartig sind (keine Ziffer von 1-9 doppelt vorkommt).
    for (int gridRow = 0; gridRow < 3; ++gridRow) {
        for (int gridCol = 0; gridCol < 3; ++gridCol) {
            std::unordered_set<int> gridCheck;
            for (int row = gridRow * 3; row < (gridRow + 1) * 3; ++row) {
                for (int col = gridCol * 3; col < (gridCol + 1) * 3; ++col) {
                    if (gridCheck.insert(board[row][col]).second) {
                        fitness++;
                    }
                }
            }
        }
    }
    // Returned den Fitnessscore eines Genoms
    return static_cast<float>(fitness);
}


// Macht ein Sudoku Feld mit 9 Zeilen a 1-9 Ziffern geshuffeld und übergib diese einem Genom
void initializer(GAGenome& g) {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    GA1DArrayGenome<int>& genome = (GA1DArrayGenome<int>&)g;

    // Für jede Zeile des Felds (insgesamt 9)
    for (int i = 0; i < 9; ++i) {
        std::vector<int> row(9); // Erschafft einen Vektor mit 1-9
        std::iota(row.begin(), row.end(), 1); // Füllt den Vektor mit den Ziffern 1-9
        std::shuffle(row.begin(), row.end(), rng); // Misch die Ziffern einer Zeile durch
        // Fügt jeden Wert der Zeile dem 1D Genom hinzu
        for (int j = 0; j < 9; ++j) {
            genome.gene(i * 9 + j, row[j]);
        }
    }
}


// Mutator function
int mutator(GAGenome& g, float p) {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    GA1DArrayGenome<int>& genome = (GA1DArrayGenome<int>&)g;

    int nMutations = 0;
    // Für jede Zeile des Felds (insgesamt 9)
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) { // Für jeden Eintrag in der Zeile
            if (GAFlipCoin(p)) {
                // Mutate mit Wahrscheinlichkeit p
                // Tauschen der Einträge in einer Zeile
                int k = GARandomInt(0, 8);
                int temp = genome.gene(i * 9 + j);
                genome.gene(i * 9 + j, genome.gene(i * 9 + k));
                genome.gene(i * 9 + k, temp);
                nMutations++;
            }
        }
    }
    return nMutations;
}

int crossover(const GAGenome& p1, const GAGenome& p2, GAGenome* c1, GAGenome* c2) {
    const GA1DArrayGenome<int>& parent1 = (const GA1DArrayGenome<int>&)p1;
    const GA1DArrayGenome<int>& parent2 = (const GA1DArrayGenome<int>&)p2;

    // Überprüfen ob es min. ein Kind zum genrieren gibt.
    if (c1 || c2) {
        // Festlegen des Crossover-Punktes
        int crossoverRow = GARandomInt(0, 8);

        if (c1) {
            GA1DArrayGenome<int>& child1 = (GA1DArrayGenome<int>&)*c1;
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    // Kopieren der Eltern (P1 bis zum Crossover Punkt dann P2)
                    child1.gene(i * 9 + j, i <= crossoverRow ? parent1.gene(i * 9 + j) : parent2.gene(i * 9 + j));
                }
            }
        }

        if (c2) {
            GA1DArrayGenome<int>& child2 = (GA1DArrayGenome<int>&)*c2;
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    // Kopieren der Eltern (P2 bis zum Crossover Punkt dann P1)
                    child2.gene(i * 9 + j, i <= crossoverRow ? parent2.gene(i * 9 + j) : parent1.gene(i * 9 + j));
                }
            }
        }

        return (c1 && c2) ? 2 : 1; // Anzahl an geschaffenen Kindern
    } else {
        return 0; // Wenn kein Kind erschaffen wurde
    }
}




// Methde die dafür zuständig ist das Ago. läuft bis ein vollständig richtiges Sudoku-Feld erstellt wurde
GABoolean terminateUponFitness(GAGeneticAlgorithm& ga) {
    float desiredFitness = EXPECTED_FITNESS;
    return (ga.statistics().maxEver() >= desiredFitness) ? gaTrue : gaFalse;
}

int main() {
    // Zeitmessung um nachzuvollziehen, wie lang die erstellung eines Feldes braucht.
    auto start = std::chrono::high_resolution_clock::now();
    srand(static_cast<unsigned int>(time(nullptr)));

    GA1DArrayGenome<int> genome(BOARD_SIZE, objective);
    genome.initializer(initializer);
    genome.mutator(mutator);
    genome.crossover(crossover);

    GASimpleGA ga(genome);
    ga.populationSize(POPULATION_SIZE);
    ga.nGenerations(MAX_GENERATIONS);
    ga.pMutation(MUTATION_RATE);
    ga.pCrossover(CROSSOVER_RATE);

    // Setzen der Regel für die Laufzeit des Algos.
    ga.terminator(terminateUponFitness);

    ga.evolve();

    // Bestes Genom (vollständiges Sudoku Feld)
    GA1DArrayGenome<int>& bestGenome = (GA1DArrayGenome<int>&)ga.statistics().bestIndividual();

    // Ausgabe des besten Feldes
    std::cout << "Berechneter Fitnessscore: " << bestGenome.score() << std::endl;
    std::cout << " -----------------------------------" << std::endl;
    std::vector<std::vector<int>> sudokuBoard(9, std::vector<int>(9));
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            sudokuBoard[i][j] = bestGenome.gene(i * 9 + j);
        }
    }
    for (int i = 0; i < 9; ++i) {
        if (i % 3 == 0) {
            std::cout << " -----------------------" << std::endl;
        }
        for (int j = 0; j < 9; ++j) {
            if (j % 3 == 0) std::cout << "| ";
            std::cout << sudokuBoard[i][j] << ' ';
        }
        std::cout << "|" << std::endl;
    }
    std::cout << " -----------------------" << std::endl;
    std::cout << " -----------------------------------" << std::endl;

    // Stoppen der zeitmessung und Ausgabe der Zeitmessung
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Benötigte Zeit: " << duration.count() << " milliseconds" << std::endl;

    return 0;
}

