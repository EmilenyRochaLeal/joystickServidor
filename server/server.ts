import express, { Request, Response } from 'express';
const app = express();

const PORT = 3000;

app.use(express.json());
app.use(express.static('public')); // para exibir a interface 

let joystickData = {
    x: 0,
    y: 0,
    botao: false,
    direcao: "Centro"
  };

app.get('/', (req: Request, res: Response)=>{
    res.send('Servidor rodando')
});

app.get('/status', (req, res) => res.json(joystickData));


// rota que recebe os dados da placa 
app.post('/receber', (req: Request, res: Response) => {
    const { x, y, botao, direcao } = req.body;
    console.log(`Recebido: x=${x}, y=${y}, botão=${botao}, direcao=${direcao}`);
    res.send('Dados recebidos com sucesso!');
});
// rota da startar a aplicação 
app.listen(PORT, () => {
    console.log(`Servidor rodando em http://localhost:${PORT}`);
});

// criar rotas necessárias para o funcionamento do projeto 