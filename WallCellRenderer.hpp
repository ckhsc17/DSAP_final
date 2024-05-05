        void RenderPassOne(
            GameRenderer<TGameConfig> &renderer,
            sf::Vector2i cellPosition,
            const IBackgroundCell<TGameConfig> &backgroundCell) const override
        {
            sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
            rectangle.setFillColor(sf::Color(60, 60, 60));
            rectangle.setPosition(sf::Vector2f(
                TGameConfig::kBoardLeft + cellPosition.x * TGameConfig::kCellSize,
                TGameConfig::kBoardTop + cellPosition.y * TGameConfig::kCellSize));
            renderer.DrawShape(rectangle);
        }